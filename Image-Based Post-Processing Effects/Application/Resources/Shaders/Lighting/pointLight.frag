#version 450 core

const float PI = 3.14159265359;

layout(location = 0) out vec4 oFragColor;

struct PointLight
{
    vec3 color;
    vec3 position;
	float radius;
	float invSqrAttRadius;
	bool renderShadows;
};

const float SHADOW_NEAR_Z = 0.1;

layout(binding = 0) uniform sampler2D uAlbedoMap;
layout(binding = 1) uniform sampler2D uNormalMap;
layout(binding = 2) uniform sampler2D uMetallicRoughnessAoMap;
layout(binding = 3) uniform sampler2D uDepthMap;
layout(binding = 14) uniform samplerCubeShadow uShadowMap;

uniform PointLight uPointLight;
uniform mat4 uInverseView;
uniform mat4 uInverseProjection;
uniform bool uShadowsEnabled;
uniform vec2 uViewportSize;

#include "brdf.h"

float smoothDistanceAtt(float squaredDistance, float invSqrAttRadius)
{
	float factor = squaredDistance * invSqrAttRadius;
	float smoothFactor = clamp(1.0 - factor * factor, 0.0, 1.0);
	return smoothFactor * smoothFactor;
}

float getDistanceAtt(vec3 unnormalizedLightVector, float invSqrAttRadius)
{
	float sqrDist = dot(unnormalizedLightVector, unnormalizedLightVector);
	float attenuation = 1.0 / (max(sqrDist, invSqrAttRadius));
	attenuation *= smoothDistanceAtt(sqrDist, invSqrAttRadius);
	
	return attenuation;
}

float vectorToDepth (vec3 vec)
{
    vec3 absVec = abs(vec);
    float localZComp = max(absVec.x, max(absVec.y, absVec.z));
	
    float f = uPointLight.radius;
    float n = SHADOW_NEAR_Z;
	
    float normZComp = (f+n) / (f-n) - (2*f*n)/(f-n)/localZComp;
    return (normZComp + 1.0) * 0.5;
}

void main()
{
	vec2 texCoord = gl_FragCoord.xy /  textureSize(uAlbedoMap, 0);
	vec4 metallicRoughnessAoShaded = texture(uMetallicRoughnessAoMap, texCoord).rgba;
	metallicRoughnessAoShaded.y *= metallicRoughnessAoShaded.y;
    
		
    if (metallicRoughnessAoShaded.a > 0.0)
    {
		float depth = texture(uDepthMap, texCoord).r;
		vec4 clipSpacePosition = vec4((gl_FragCoord.xy / uViewportSize) * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
		vec4 viewSpacePosition = uInverseProjection * clipSpacePosition;
		viewSpacePosition /= viewSpacePosition.w;

		vec3 unnormalizedLightVector = uPointLight.position - viewSpacePosition.xyz;;
		vec3 L = normalize(unnormalizedLightVector);
		float att = getDistanceAtt(unnormalizedLightVector, uPointLight.invSqrAttRadius);

		vec3 N = texture(uNormalMap, texCoord).xyz;
		vec3 V = -normalize(viewSpacePosition.xyz);
		vec3 R = reflect(-V, N);
		vec3 H = normalize(V + L);

		float NdotV = max(dot(N, V), 0.0);
		float NdotL = max(dot(N, L), 0.0);

		// Cook-Torrance BRDF
		float NDF = DistributionGGX(N, H, metallicRoughnessAoShaded.g);
		float G = GeometrySmith(NdotV, NdotL, metallicRoughnessAoShaded.g);

		vec3 albedo = texture(uAlbedoMap, texCoord).rgb;
		vec3 F0 = mix(vec3(0.04), albedo, metallicRoughnessAoShaded.r);

		vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

		vec3 nominator = NDF * G * F;
		float denominator = max(4 * NdotV * NdotL, 0.0000001);

		vec3 specular = nominator / denominator;

		// because of energy conversion kD and kS must add up to 1.0
		vec3 kD = vec3(1.0) - F;
		// multiply kD by the inverse metalness so if a material is metallic, it has no diffuse lighting (and otherwise a blend)
		kD *= 1.0 - metallicRoughnessAoShaded.r;

		vec3 radiance = uPointLight.color * att;
		oFragColor = metallicRoughnessAoShaded.a * vec4((kD * albedo.rgb / PI + specular) * radiance * NdotL, 1.0);

		if(uPointLight.renderShadows && uShadowsEnabled)
		{
			vec3 lightToFrag = viewSpacePosition.xyz + 0.1 * L - uPointLight.position;
			lightToFrag = (uInverseView * vec4(lightToFrag, 0.0)).xyz;
			//lightToFrag -= normalize(lightToFrag) * 0.1;
			float fragDepth = vectorToDepth(lightToFrag);
			float shadow = texture(uShadowMap, vec4(lightToFrag, fragDepth)).x;
			oFragColor.rgb *= (1.0 - shadow);
		}
    }
	else
	{
		oFragColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
}