#version 450 core

const float PI = 3.14159265359;

layout(location = 0) out vec4 oFragColor;

in vec2 vTexCoord;

struct SpotLight
{
    vec3 color;
    vec3 position;
	vec3 direction;
	float angleScale;
	float angleOffset;
	float invSqrAttRadius;
	bool renderShadows;
	bool projector;
	
	mat4 viewProjectionMatrix;
};


layout(binding = 0) uniform sampler2D uAlbedoMap;
layout(binding = 1) uniform sampler2D uNormalMap;
layout(binding = 2) uniform sampler2D uMetallicRoughnessAoMap;
layout(binding = 3) uniform sampler2D uDepthMap;
layout(binding = 14) uniform sampler2DShadow uShadowMap;
layout(binding = 5) uniform sampler2D uProjectionMap;

uniform SpotLight uSpotLight;
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

float getAngleAtt(vec3 normalizedLightVector, vec3 lightDir, float lightAngleScale, float lightAngleOffset)
{
	// On the CPU
	// float lightAngleScale = 1.0f / max(0.001f, (cosInner - cosOuter));
	// float lightAngleOffset = -cosOuter * angleScale;

	float cd = dot(lightDir, normalizedLightVector);
	float attenuation = clamp(cd * lightAngleScale + lightAngleOffset, 0.0, 1.0);
	// smooth the transition
	attenuation *= attenuation;
	
	return attenuation;
}


void main()
{
	vec2 texCoord = gl_FragCoord.xy /  textureSize(uAlbedoMap, 0);
    
	vec4 metallicRoughnessAoShaded = texture(uMetallicRoughnessAoMap, texCoord).rgba;
    
    if (metallicRoughnessAoShaded.a > 0.0)
    {
		float depth = texture(uDepthMap, texCoord).r;
		vec4 clipSpacePosition = vec4((gl_FragCoord.xy / uViewportSize) * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
		vec4 viewSpacePosition = uInverseProjection * clipSpacePosition;
		viewSpacePosition /= viewSpacePosition.w;

		vec3 unnormalizedLightVector = uSpotLight.position - viewSpacePosition.xyz;;
		vec3 L = normalize(unnormalizedLightVector);
		float att = 1.0;
		att *= getDistanceAtt(unnormalizedLightVector, uSpotLight.invSqrAttRadius);
		att *= getAngleAtt(-L, uSpotLight.direction, uSpotLight.angleScale, uSpotLight.angleOffset);

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

		vec3 radiance = uSpotLight.color * att;
		oFragColor = vec4((kD * albedo.rgb / PI + specular) * radiance * NdotL , 1.0);

		if(uSpotLight.renderShadows && uShadowsEnabled || uSpotLight.projector)
		{
			vec4 worldPos4 = uInverseView * (viewSpacePosition + 0.1 * vec4(L, 0.0));
			vec4 projCoords4 = uSpotLight.viewProjectionMatrix * worldPos4;
			vec3 projCoords = projCoords4.xyz / projCoords4.w;
			projCoords = projCoords * 0.5 + 0.5; 

			float shadow = 0.0;

			if(uSpotLight.renderShadows && uShadowsEnabled)
			{
				vec2 invShadowMapSize = vec2(1.0 / (textureSize(uShadowMap, 0).xy));

				float count = 0.0;
				float radius = 2.0;
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

			oFragColor.rgb *= (1.0 - shadow) * projectedColor;
		}
    }
	else
	{
		oFragColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
}