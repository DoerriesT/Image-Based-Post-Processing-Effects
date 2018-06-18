#version 450 core

layout(location = 0) out vec4 oAlbedo;

in vec2 vTexCoord;
in vec3 vWorldPos;

uniform sampler2D uNormalTexture;
uniform sampler2D uDisplacementTexture;
uniform sampler2D uFoamTexture;
uniform samplerCube uEnvironmentTexture;
uniform float uWaterLevel;
uniform vec3 uCamPos;
uniform mat4 uView;
uniform vec2 uTexCoordShift;
uniform bool uUseEnvironment;
uniform vec3 uLightDir;
uniform vec3 uLightColor;

vec3 decode(vec2 enc)
{
    vec2 fenc = enc * 4.0 - 2.0;
    float f = dot(fenc, fenc);
    float g = sqrt(1.0 - f * 0.25);
    vec3 n;
    n.xy = fenc * g;
    n.z = 1.0 -f * 0.5;
    return n;
}

void main()
{
	vec3 V = normalize(uCamPos - vWorldPos);
	vec3 L = uLightDir;
	vec3 N = normalize(decode(texture(uNormalTexture, vTexCoord).rg));

	// only the crests of water waves generate double refracted light
	float scatterFactor = smoothstep(0.0, 1.0, 2.5 * max(0.0, (vWorldPos.y - uWaterLevel)));

	// the waves that lie between camera and light projection on water plane generate maximal amount of double refracted light 
	float tmp = max(0.0, dot(normalize(vec3(L.x, 0.0, L.z)), -V));
	scatterFactor *= tmp * tmp;
	
	// the slopes of waves that are oriented back to light generate maximal amount of double refracted light 
	tmp = max(0.0, 1.0 - dot(L, N));
	scatterFactor *= ((tmp * tmp) * (tmp * tmp)) * ((tmp * tmp) * (tmp * tmp));
	
	// water crests gather more light than lobes, so more light is scattered under the crests
	scatterFactor += 1.5 * max(0, (vWorldPos.y - uWaterLevel) * 2.0 + 1.0)
	// the scattered light is best seen if observing direction is normal to slope surface
	* max(0.0, dot(V, N))
	// fading scattered light out at distance and if viewing direction is vertical to avoid unnatural look
	* max(0.0, 1.0 - V.y) * (300.0/(300 + length(uCamPos - vWorldPos)));
	scatterFactor *= max(0.0, 1.0 - dot(N, L)) * max(0.0, 1.0 - dot(N, V));

	// calculating fresnel factor 
	float r=(1.2-1.0)/(1.2+1.0);
	float fresnelFactor = clamp(r+(1.0-r) * pow(1.0 - max(dot(N, V), 0.0), 5.0), 0.0, 1.0);

	// calculating specular factor
	vec3 R = reflect(-V, N);
	float specularFactor = fresnelFactor * pow(max(0.0, dot(L, R)), 1000.0);

	// calculating diffuse intensity of water surface itself
	float diffuseFactor = 0.1 + 0.2 * max(0.0, dot(L, N));

	// getting reflection and refraction color at disturbed texture coordinates
	vec3 reflectionColor = uUseEnvironment ? textureLod(uEnvironmentTexture, R, 0).rgb : vec3(1.0);

	// calculating water surface color and applying atmospheric fog to it
	vec3 waterColor = uLightColor * vec3(0.1, 0.4, 0.9) * diffuseFactor * 0.25;
	
	// combining final water color
	vec3 color = mix(waterColor.rgb, reflectionColor.rgb, fresnelFactor);
	color += 350.0 * specularFactor * fresnelFactor * uLightColor;
	color += vec3(0.3, 0.7, 0.6) * scatterFactor * 0.1;
	color *= 0.1;

	oAlbedo = vec4(color, 1.0);
}