#version 450 core

#ifndef SSAO_ENABLED
#define SSAO_ENABLED 0
#endif // SSAO_ENABLED

#ifndef MAX_REFLECTION_LOD
#define MAX_REFLECTION_LOD 4.0
#endif // MAX_REFLECTION_LOD

#define PI 3.14159265359

layout(location = 0) out vec4 oFragColor;

in vec2 vTexCoord;

layout(binding = 0) uniform sampler2D uAlbedoTexture;
layout(binding = 1) uniform sampler2D uNormalTexture;
layout(binding = 2) uniform sampler2D uMetallicRoughnessAoTexture;
layout(binding = 3) uniform sampler2D uDepthTexture;
layout(binding = 4) uniform sampler2D uSsaoTexture;
layout(binding = 9) uniform sampler2D uBrdfLUT;
layout(binding = 10) uniform sampler2D uReflectionTexture;

uniform mat4 uInverseView;
uniform mat4 uInverseProjection;
uniform vec3 uBoxMin;
uniform vec3 uBoxMax;
uniform vec3 uProbePosition;

#include "brdf.h"

float computeDistanceBasedRoughness(
	float distIntersectionToShadedPoint,
	float distIntersectionToProbeCenter,
	float linearRoughness)
{
	// To avoid artifacts we clamp to the original linearRoughness
	// which introduces an acceptable bias and allows conservation
	// of mirror reflection behavior for a smooth surface
	float newLinearRoughness = clamp(distIntersectionToShadedPoint / 
		distIntersectionToProbeCenter * linearRoughness, 0.0, linearRoughness);
	return mix(newLinearRoughness, linearRoughness, linearRoughness);
}

vec3 parallaxCorrect(vec3 R, vec3 P, inout float roughness)
{
	const vec3 boxMin = uBoxMin;
	const vec3 boxMax = uBoxMax;
	const vec3 probePos = uProbePosition;
	
	const vec3 firstPlaneIntersect = (boxMax - P) / R;
	const vec3 secondPlaneIntersect = (boxMin - P) / R;
	
	const vec3 furthestPlane = max(firstPlaneIntersect, secondPlaneIntersect);

	const float dist = min(min(furthestPlane.x, furthestPlane.y), furthestPlane.z);
	
	const vec3 intersectPos = P + R * dist;
	vec3 probeToIntersection = intersectPos - probePos;
	const float distProbeToIntersection = length(probeToIntersection);

	roughness = computeDistanceBasedRoughness(dist, distProbeToIntersection, roughness);

	return probeToIntersection * (1.0 / distProbeToIntersection);
}

vec2 signNotZero(vec2 v) 
{
	return vec2((v.x >= 0.0) ? +1.0 : -1.0, (v.y >= 0.0) ? +1.0 : -1.0);
}

/** Assumes that v is a unit vector. The result is an octahedral vector on the [-1, +1] square. */
vec2 octEncode(in vec3 v) 
{
    const float l1norm = abs(v.x) + abs(v.y) + abs(v.z);
    vec2 result = v.xy * (1.0 / l1norm);
    if (v.z < 0.0) {
        result = (1.0 - abs(result.yx)) * signNotZero(result.xy);
    }
    return result;
}

float computeSpecularOcclusion(float NdotV, float ao, float roughness)
{
	return clamp(pow(NdotV + ao, exp2(-16.0 * roughness - 1.0)) - 1.0 + ao, 0.0, 1.0);
}

void main()
{
	oFragColor = vec4(0.0, 0.0, 0.0, 1.0);

	const vec2 texCoord = gl_FragCoord.xy / textureSize(uAlbedoTexture, 0);

	vec4 metallicRoughnessAoShaded = texture(uMetallicRoughnessAoTexture, texCoord).rgba;

#if SSAO_ENABLED
	metallicRoughnessAoShaded.z = min(metallicRoughnessAoShaded.z, texture(uSsaoTexture, texCoord).r);
#endif // SSAO_ENABLED

	const float depth = texture(uDepthTexture, texCoord).r;
	const vec4 clipSpacePosition = vec4(vec3(texCoord, depth) * 2.0 - 1.0, 1.0);
	vec4 viewSpacePosition = uInverseProjection * clipSpacePosition;
	viewSpacePosition /= viewSpacePosition.w;
	
	const vec4 worldPos4 = uInverseView * viewSpacePosition;
	const vec3 worldPos = worldPos4.xyz / worldPos4.w;

	if (metallicRoughnessAoShaded.w == 0.0
	|| any(greaterThan(worldPos, uBoxMax))
	|| any(lessThan(worldPos, uBoxMin)))
	{
		discard;
	}
    	
	const vec3 V = -normalize(viewSpacePosition.xyz);
	const vec3 N = texture(uNormalTexture, texCoord).xyz;
	const float NdotV = max(dot(N, V), 0.0);

	float roughness = metallicRoughnessAoShaded.y;
	const vec3 correctedTexCoord = parallaxCorrect((uInverseView * vec4(reflect(-V, N), 0.0)).xyz, worldPos, roughness);
	
	// ambient lighting using IBL
	const vec3 albedo = texture(uAlbedoTexture, texCoord).rgb;
	const vec3 F0 = mix(vec3(0.04), albedo, metallicRoughnessAoShaded.x);
	const vec3 kS = fresnelSchlickRoughness(NdotV, F0, roughness);

	
	
	// sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
	const vec3 prefilteredColor = textureLod(uReflectionTexture, octEncode(correctedTexCoord) * 0.5 + 0.5, roughness * MAX_REFLECTION_LOD).rgb;

	const vec2 brdf  = texture(uBrdfLUT, vec2(NdotV, roughness)).rg;
	const vec3 specular = prefilteredColor * (kS * brdf.x + brdf.y);

	oFragColor.rgb = specular * computeSpecularOcclusion(NdotV, metallicRoughnessAoShaded.z, roughness);
}