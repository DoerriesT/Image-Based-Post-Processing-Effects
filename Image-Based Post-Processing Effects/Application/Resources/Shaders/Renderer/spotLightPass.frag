#version 330 core

const float PI = 3.14159265359;

layout(location = 0) out vec4 oFragColor;

in vec2 vTexCoord;

struct SpotLight
{
    vec3 color;
    vec3 position;
	vec3 direction;
	float angle;
	bool renderShadows;
	sampler2D shadowMap;
	mat4 viewProjectionMatrix;
};


uniform sampler2D uAlbedoMap;
uniform sampler2D uNormalMap;
uniform sampler2D uMetallicRoughnessAoMap;
uniform sampler2D uDepthMap;
uniform SpotLight uSpotLight;
uniform mat4 uInverseView;
uniform mat4 uInverseProjection;
uniform vec3 uCamPos;
uniform bool uShadowsEnabled;
uniform vec2 uViewportSize;

vec3 decode (vec2 enc)
{
    vec2 fenc = enc * 4.0 - 2.0;
    float f = dot(fenc, fenc);
    float g = sqrt(1.0 - f * 0.25);
    vec3 n;
    n.xy = fenc * g;
    n.z = 1.0 -f * 0.5;
    return n;
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a2 = roughness*roughness;
    a2 *= a2;
    float NdotH2 = max(dot(N, H), 0.0);
    NdotH2 *= NdotH2;

    float nom   = a2;
    float denom = max(NdotH2 * (a2 - 1.0) + 1.0, 0.0000001);

    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySmith(float NdotV, float NdotL, float roughness)
{
	float r = (roughness + 1.0);
    float k = (r*r) / 8.0;
    float ggx2 =  NdotV / max(NdotV * (1.0 - k) + k, 0.0000001);
    float ggx1 = NdotL / max(NdotL * (1.0 - k) + k, 0.0000001);

    return ggx1 * ggx2;
}


vec3 fresnelSchlick(float HdotV, vec3 F0)
{
	float fresnel = 1.0 - HdotV;
	fresnel *= fresnel;
	fresnel *= fresnel;
	return F0 + (1.0 - F0) * fresnel;
}

void main()
{
	vec2 texCoord = gl_FragCoord.xy /  textureSize(uAlbedoMap, 0);
    vec3 albedo = texture(uAlbedoMap, texCoord).rgb;
	
	vec4 metallicRoughnessAoShaded = texture(uMetallicRoughnessAoMap, texCoord).rgba;
	
	float depth = texture(uDepthMap, texCoord).r;
	vec4 clipSpacePosition = vec4((gl_FragCoord.xy / uViewportSize) * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
	vec4 viewSpacePosition = uInverseProjection * clipSpacePosition;
	viewSpacePosition /= viewSpacePosition.w;
	vec4 worldPos4 = uInverseView * viewSpacePosition;
	vec3 worldPos = worldPos4.xyz;

	
	vec3 L = uSpotLight.position - worldPos;
	float distance = length(L);
	L /= distance;
    
    if (metallicRoughnessAoShaded.a > 0.0 && dot(-L, uSpotLight.direction) >= uSpotLight.angle)
    {
		vec3 N = decode(texture(uNormalMap, texCoord).xy);
		
				
		vec3 V = normalize(uCamPos - worldPos);
		vec3 R = reflect(-V, N);
		vec3 H = normalize(V + L);

		vec3 F0 = mix(vec3(0.04), albedo, metallicRoughnessAoShaded.r);

		float NdotV = max(dot(N, V), 0.0);
		float NdotL = max(dot(N, L), 0.0);
			
		// shadow
		float shadow = 0.0;
		if(uSpotLight.renderShadows && uShadowsEnabled)
		{
					
			vec4 projCoords4 = uSpotLight.viewProjectionMatrix * worldPos4;
			vec3 projCoords = (projCoords4 / projCoords4.w).xyz;
			projCoords = projCoords * 0.5 + 0.5; 
			vec2 moments = texture(uSpotLight.shadowMap, projCoords.xy).xy;
			float currentDepth = projCoords.z;

			float p = (currentDepth <= moments.x) ? 1.0 : 0.0;
			float variance = moments.y - (moments.x * moments.x);
			variance = max(variance, 0.00001);
			float d = currentDepth - moments.x;
			float p_max = variance / (variance + d * d);
			shadow = 1.0 - max(p, p_max);
			if(projCoords.z > 1.0)
			{
				shadow = 0.0;
			}
		}


		float attenuation = 1.0 / (distance * distance);
		vec3 radiance = uSpotLight.color * attenuation;

		// Cook-Torrance BRDF
		float NDF = DistributionGGX(N, H, metallicRoughnessAoShaded.g);
		float G = GeometrySmith(NdotV, NdotL, metallicRoughnessAoShaded.g);
		vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

		vec3 nominator = NDF * G * F;
		float denominator = max(4 * NdotV * NdotL, 0.0000001);

		vec3 specular = nominator / denominator;

		// because of energy conversion kD and kS must add up to 1.0
		vec3 kD = vec3(1.0) - F;
		// multiply kD by the inverse metalness so if a material is metallic, it has no diffuse lighting (and otherwise a blend)
		kD *= 1.0 - metallicRoughnessAoShaded.r;

		oFragColor = metallicRoughnessAoShaded.a * vec4((kD * albedo.rgb / PI + specular) * radiance * NdotL * (1.0 - shadow), 1.0);
    }
	else
	{
		oFragColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
}