#version 450 core

const float PI = 3.14159265359;

layout(location = 0) out vec4 oFragColor;

in vec2 texCoord;

struct SpotLight
{
    vec3 color;
    vec3 position;
	vec3 direction;
	float outerAngle;
	float innerAngle;
	float radius;
	bool renderShadows;
	bool projector;
	
	mat4 viewProjectionMatrix;
};


layout(binding = 0) uniform sampler2D uAlbedoRMTexture;
layout(binding = 1) uniform sampler2D uNormalAoTexture;
layout(binding = 3) uniform sampler2D uDepthMap;
layout(binding = 14) uniform sampler2DShadow uShadowMap;
layout(binding = 5) uniform sampler2D uProjectionMap;

uniform SpotLight uSpotLight;
uniform mat4 uInverseView;
uniform mat4 uInverseProjection;
uniform bool uShadowsEnabled;
uniform vec2 uViewportSize;

#include "brdf.h"

vec2 signNotZero(vec2 v) 
{
	return vec2((v.x >= 0.0) ? +1.0 : -1.0, (v.y >= 0.0) ? +1.0 : -1.0);
}

/** Returns a unit vector. Argument o is an octahedral vector packed via octEncode,
    on the [-1, +1] square*/
vec3 octDecode(vec2 o) 
{
    vec3 v = vec3(o.x, o.y, 1.0 - abs(o.x) - abs(o.y));
    if (v.z < 0.0) 
	{
        v.xy = (1.0 - abs(v.yx)) * signNotZero(v.xy);
    }
    return normalize(v);
}

vec2 unorm8x3ToSnorm12x2(vec3 u)
{
	u *= 255.0;
	u.y *= (1.0 / 16.0);
	vec2 s = vec2(u.x * 16.0 + floor(u.y), fract(u.y) * (16.0 * 256.0) + u.z);
	return clamp(s * (1.0 / 2047.0) - 1.0, vec2(-1.0), vec2(1.0));
}

vec3 YCoCg2RGB(vec3 c)
{
	c.y -= 0.5;
	c.z -= 0.5;
	return vec3(c.r + c.g - c.b, c.r + c.b, c.r - c.g - c.b);
}

const float THRESH=30./255.;

float edgeFilter(vec2 center, vec2 a0, vec2 a1, vec2 a2, vec2 a3)
{ 
	vec4 lum = vec4(a0.x, a1.x , a2.x, a3.x);
	vec4 w = 1.0 - step(THRESH, abs(lum - center.x)); 
	float W = w.x + w.y + w.z + w.w;
	//Handle the special case where all the weights are zero.
	//In HDR scenes it's better to set the chrominance to zero. 
	//Here we just use the chrominance of the first neighbor.
	w.x = (W == 0.0) ? 1.0 : w.x;  
	W = (W == 0.0) ? 1.0 : W;  

	return (w.x * a0.y + w.y * a1.y + w.z * a2.y + w.w * a3.y) * (1.0 / W);
}

void main()
{	
	const vec2 texelSize = 1.0 / textureSize(uAlbedoRMTexture, 0);
	const vec2 texCoord = gl_FragCoord.xy * texelSize;

	const float depth = texture(uDepthMap, texCoord).r;
	vec4 clipSpacePosition = vec4((gl_FragCoord.xy / uViewportSize) * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
	vec4 viewSpacePosition = uInverseProjection * clipSpacePosition;
	viewSpacePosition /= viewSpacePosition.w;

	const vec4 albedoRM = texture(uAlbedoRMTexture, texCoord).xyzw;

	const vec2 a0 = texture(uAlbedoRMTexture, texCoord + vec2(texelSize.x, 0.0)).rg;
	const vec2 a1 = texture(uAlbedoRMTexture, texCoord - vec2(texelSize.x, 0.0)).rg;
	const vec2 a2 = texture(uAlbedoRMTexture, texCoord + vec2(0.0, texelSize.y)).rg;
	const vec2 a3 = texture(uAlbedoRMTexture, texCoord - vec2(0.0, texelSize.y)).rg;	
	const float chroma = edgeFilter(albedoRM.rg, a0, a1, a2, a3);

	const ivec2 crd = ivec2(gl_FragCoord.xy);
	const bool pattern = ((crd.x & 1) == (crd.y & 1));
	
	vec3 albedo = vec3(albedoRM.rg, chroma);
	albedo = YCoCg2RGB(pattern ? albedo.rbg : albedo.rgb);
	albedo = pow(albedo, vec3(2.2));

	const float metallic = albedoRM.w;
	const float roughness = albedoRM.z;

	const vec3 V = -normalize(viewSpacePosition.xyz);
	const vec3 N = octDecode(unorm8x3ToSnorm12x2(texture(uNormalAoTexture, texCoord).xyz));

	vec3 L = uSpotLight.position - viewSpacePosition.xyz;
	float distance = length(L);
	L /= distance;

	const float NdotV = max(dot(N, V), 0.0);
	const float NdotL = max(dot(N, L), 0.0);
	const vec3 H = normalize(V + L);

	// Cook-Torrance BRDF
	const float NDF = DistributionGGX(N, H, roughness);
	const float G = GeometrySmith(NdotV, NdotL, roughness);
	const vec3 F0 = mix(vec3(0.04), albedo, metallic);
	const vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

	const vec3 nominator = NDF * G * F;
	const float denominator = max(4 * NdotV * NdotL, 0.0000001);

	const vec3 specular = nominator / denominator;

	// because of energy conversion kD and kS must add up to 1.0
	vec3 kD = vec3(1.0) - F;
	// multiply kD by the inverse metalness so if a material is metallic, it has no diffuse lighting (and otherwise a blend)
	kD *= 1.0 - metallic;

	oFragColor = vec4((kD * albedo.rgb / PI + specular) * NdotL, 1.0);

	const float distancePercentage = distance / uSpotLight.radius;
	float attenuation = clamp(1.0 - (distancePercentage * distancePercentage * distancePercentage * distancePercentage), 0.0, 1.0);
	attenuation *= attenuation;
	attenuation /= distance * distance + 1.0;

	const float innerAngle = 1.0 - uSpotLight.innerAngle;
	const float outerAngle = 1.0 - uSpotLight.outerAngle;
	float angle = 1.0 - dot(-L, uSpotLight.direction);
	angle = clamp(angle - innerAngle, 0.0, outerAngle - innerAngle);
	const float blendWeight = (outerAngle == innerAngle) ? 1.0 : 1.0 - angle / (outerAngle - innerAngle);
	attenuation *= smoothstep(0.0, 1.0, blendWeight);
	
	const vec3 radiance = uSpotLight.color * attenuation;
	oFragColor.rgb *= radiance;

	if(uSpotLight.renderShadows && uShadowsEnabled || uSpotLight.projector)
	{
		const vec4 worldPos4 = uInverseView * (viewSpacePosition + 0.1 * vec4(L, 0.0));
		const vec4 projCoords4 = uSpotLight.viewProjectionMatrix * worldPos4;
		vec3 projCoords = projCoords4.xyz / projCoords4.w;
		projCoords = projCoords * 0.5 + 0.5; 

		float shadow = 0.0;

		if(uSpotLight.renderShadows && uShadowsEnabled)
		{
			const vec2 invShadowMapSize = vec2(1.0 / (textureSize(uShadowMap, 0).xy));

			float count = 0.0;
			const float radius = 2.0;
			for(float row = -radius; row <= radius; ++row)
			{
				for(float col = -radius; col <= radius; ++col)
				{
					++count;
					shadow += texture(uShadowMap, vec3(projCoords.xy + vec2(col, row) * invShadowMapSize, projCoords.z)).x;
				}
			}
			shadow *= 1.0 / count;
		}

		vec3 projectedColor = vec3(1.0);

		if (uSpotLight.projector)
		{
			projectedColor = texture(uProjectionMap, vec2(projCoords.x, 1.0 - projCoords.y)).rgb;
		}

		// assuming there is only the spot light contribution in oFragColor.rgb
		oFragColor.rgb *= (1.0 - shadow) * projectedColor;
	}
}