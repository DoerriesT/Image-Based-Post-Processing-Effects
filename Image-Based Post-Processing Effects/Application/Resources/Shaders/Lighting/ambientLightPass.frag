#version 450 core

#ifndef SSAO_ENABLED
#define SSAO_ENABLED 0
#endif // SSAO_ENABLED

#ifndef SSR_ENABLED
#define SSR_ENABLED 0
#endif // SSR_ENABLED

#ifndef IRRADIANCE_VOLUME
#define IRRADIANCE_VOLUME 0
#endif // IRRADIANCE_VOLUME

layout(location = 0) out vec4 oFragColor;

in vec2 vTexCoord;

layout(binding = 0) uniform sampler2D uAlbedoMap;
layout(binding = 1) uniform sampler2D uNormalMap;
layout(binding = 2) uniform sampler2D uMetallicRoughnessAoMap;
layout(binding = 3) uniform sampler2D uDepthMap;
layout(binding = 4) uniform sampler2D uSsaoMap;
layout(binding = 9) uniform sampler2D uBrdfLUT;
layout(binding = 10) uniform sampler2D uPrevFrame;
layout(binding = 12) uniform sampler2D uIrradianceMap;
layout(binding = 13) uniform sampler2D uPrefilterMap;

uniform mat4 uInverseView;
uniform mat4 uInverseProjection;
#if SSR_ENABLED
uniform mat4 uProjection;
uniform mat4 uReProjection;
#endif // SSR_ENABLED

#if IRRADIANCE_VOLUME
uniform vec3 uVolumeOrigin;
uniform ivec3 uVolumeDimensions;
uniform float uSpacing;
#endif // IRRADIANCE_VOLUME

const float PI = 3.14159265359;
const float MAX_REFLECTION_LOD = 4.0;
const float Z_NEAR = 0.1;
const float Z_FAR = 3000.0;

#include "brdf.h"



#define INVALID_HIT_POINT vec3(-1.0)
#define LRM_MAX_ITERATIONS		100						// Maximal number of iterations for linear ray-marching
#define BS_MAX_ITERATIONS		30						// Maximal number of iterations for bineary search
#define BS_DELTA_EPSILON 0.0001

#if IRRADIANCE_VOLUME
vec3 sphericalHarmonicsIrradiance(vec3 P, vec3 N)
{
	const vec3 minCorner = uVolumeOrigin;
	const vec3 maxCorner = vec3(uVolumeDimensions - ivec3(1)) * uSpacing.xxx + minCorner;
	P = clamp(P, minCorner, maxCorner);
	
	const ivec3 baseGridCoord = ivec3((P - minCorner) / uSpacing);

	float totalWeight = 0.0;

	vec3 irradiance = vec3(0.0);
	
	for (int i = 0; i < 8; ++i)
	{
		const ivec3 offset = ivec3(i, i >> 1, i >> 2) & ivec3(1);
		const ivec3 probeCoord = baseGridCoord + offset;
		const vec3 probePos = uSpacing * vec3(probeCoord) + minCorner;
		const vec3 toProbe = probePos - P;
		const vec3 alpha = 1.0 - (abs(toProbe) / uSpacing);
		const float weight = alpha.x * alpha.y * alpha.z * float(probeCoord == clamp(probeCoord, ivec3(0), uVolumeDimensions - ivec3(1))) * max(0.005, dot(normalize(toProbe), N));
		totalWeight += weight;

		const int index = probeCoord.z * (uVolumeDimensions.x * uVolumeDimensions.y) + probeCoord.y * uVolumeDimensions.x + probeCoord.x;

		irradiance += weight * texelFetch(uIrradianceMap, ivec2(0, index), 0).rgb * 0.282095f;
		irradiance += weight * texelFetch(uIrradianceMap, ivec2(1, index), 0).rgb * 0.488603f * N.y;
		irradiance += weight * texelFetch(uIrradianceMap, ivec2(2, index), 0).rgb * 0.488603f * N.z;
		irradiance += weight * texelFetch(uIrradianceMap, ivec2(3, index), 0).rgb * 0.488603f * N.x;
		irradiance += weight * texelFetch(uIrradianceMap, ivec2(4, index), 0).rgb * 1.092548f * N.x * N.y;
		irradiance += weight * texelFetch(uIrradianceMap, ivec2(5, index), 0).rgb * 1.092548f * N.y * N.z;
		irradiance += weight * texelFetch(uIrradianceMap, ivec2(6, index), 0).rgb * 0.315392f * (3.0f * N.z * N.z - 1.0f);
		irradiance += weight * texelFetch(uIrradianceMap, ivec2(7, index), 0).rgb * 1.092548f * N.x * N.z;
		irradiance += weight * texelFetch(uIrradianceMap, ivec2(8, index), 0).rgb * 0.546274f * (N.x * N.x - N.y * N.y);
	}

    return irradiance * (1.0 / totalWeight);
}
#endif // IRRADIANCE_VOLUME

vec3 parallaxCorrect(vec3 R, vec3 P)
{
	const vec3 boxMin = vec3(-9.5, -0.01, -2.4);
	const vec3 boxMax = vec3(9.5, 15.0, 2.4);
	const vec3 probePos = vec3(0.0, 2.0, 0.0);
	
	const vec3 firstPlaneIntersect = (boxMax - P) / R;
	const vec3 secondPlaneIntersect = (boxMin - P) / R;
	
	const vec3 furthestPlane = max(firstPlaneIntersect, secondPlaneIntersect);

	const float dist = min(min(furthestPlane.x, furthestPlane.y), furthestPlane.z);
	
	const vec3 intersectPos = P + R * dist;

	return (P.x >= boxMin.x && P.y >= boxMin.y && P.z >= boxMin.z && P.x <= boxMax.x && P.y <= boxMax.y && P.z <= boxMax.z) ? normalize(intersectPos - probePos) : R;
}

float linearDepth(float depth)
{
    const float z_n = 2.0 * depth - 1.0;
    return 2.0 * Z_NEAR * Z_FAR / (Z_FAR + Z_NEAR - z_n * (Z_FAR - Z_NEAR));
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

#if SSR_ENABLED
vec3 project(vec3 vsCoord)
{
	vec4 projectedCoord = uProjection * vec4(vsCoord, 1.0f);
	projectedCoord.xyz /= projectedCoord.w;
	projectedCoord.xy = projectedCoord.xy * vec2(0.5) + vec2(0.5);

	return projectedCoord.xyz;
}

vec3 unproject(vec3 ssCoord)
{
	ssCoord.xy = (ssCoord.xy - vec2(0.5)) * vec2(2.0);
	const vec4 vsCoord = uInverseProjection * vec4(ssCoord, 1.0);
	return vsCoord.xyz / vsCoord.w;
}

bool linearRayTraceBinarySearch(inout vec3 rayPos, vec3 rayDir)
{
	for (uint i = 0; i < BS_MAX_ITERATIONS; ++i)
	{		
		/* Check if we found our final hit point */
		const float depth = textureLod(uDepthMap, rayPos.xy, 0.0).r;
		const float depthDelta = linearDepth(depth) - linearDepth(rayPos.z);
		
		if (abs(depthDelta) < BS_DELTA_EPSILON)
		{
			return true;
		}
		
		/*
		Move ray forwards if we are in front of geometry and
		move backwards, if we are behind geometry
		*/
		if (depthDelta > 0.0)
		{
			rayPos += rayDir;
		}
		else
		{
			rayPos -= rayDir;
		}
		
		rayDir *= 0.5;
	}
	return false;
}

bool linearRayTrace(inout vec3 rayPos, vec3 rayDir)
{
	rayDir = normalize(rayDir);
	const vec3 rayStart = rayPos;
	const vec3 rayEnd = rayStart + rayDir;
	vec3 prevPos = rayPos;
	
	
	for (uint i = 0; i < LRM_MAX_ITERATIONS; ++i)
	{
		/* Move to the next sample point */
		prevPos = rayPos;
		rayPos = mix(rayStart, rayEnd, float(i) / LRM_MAX_ITERATIONS);
		
		/* Check if the ray hit any geometry (delta < 0) */
		float depth = textureLod(uDepthMap, rayPos.xy, 0).r;
		float depthDelta = depth - rayPos.z;
		
		if (depthDelta < 0.0)
		{			
			/*
			Move between the current and previous point and
			make a binary search, to quickly find the final hit point
			*/
			rayPos = (rayPos + prevPos) * 0.5;
			return linearRayTraceBinarySearch(rayPos, (rayPos - prevPos) );
		}
	}
	
	rayPos = INVALID_HIT_POINT;
	return false;
}
#endif // SSR_ENABLED

vec3 decode (vec2 enc)
{
    const vec2 fenc = enc * 4.0 - 2.0;
    const float f = dot(fenc, fenc);
    const float g = sqrt(1.0 - f * 0.25);
    vec3 n;
    n.xy = fenc * g;
    n.z = 1.0 -f * 0.5;
    return n;
}

void main()
{
	const vec2 texCoord = gl_FragCoord.xy / textureSize(uAlbedoMap, 0);

	const vec3 albedo = texture(uAlbedoMap, texCoord).rgb;
	
#if SSAO_ENABLED
	vec4 metallicRoughnessAoShaded = texture(uMetallicRoughnessAoMap, texCoord).rgba;
	metallicRoughnessAoShaded.b = min(metallicRoughnessAoShaded.b, texture(uSsaoMap, texCoord).r);
#else
	const vec4 metallicRoughnessAoShaded = texture(uMetallicRoughnessAoMap, texCoord).rgba;
#endif // SSAO_ENABLED
    
		
    if (metallicRoughnessAoShaded.a > 0.0)
    {
		const vec3 N = texture(uNormalMap, texCoord).xyz;//decode(texture(uNormalMap, texCoord).xy);
		
		const float depth = texture(uDepthMap, texCoord).r;
		const vec4 clipSpacePosition = vec4(vTexCoord * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
		vec4 viewSpacePosition = uInverseProjection * clipSpacePosition;
		viewSpacePosition /= viewSpacePosition.w;

		const vec3 F0 = mix(vec3(0.04), albedo, metallicRoughnessAoShaded.r);
		const vec3 V = -normalize(viewSpacePosition.xyz);
		const float NdotV = max(dot(N, V), 0.0);
		
		// ambient lighting using IBL
		const vec3 kS = fresnelSchlickRoughness(NdotV, F0, metallicRoughnessAoShaded.g);
		vec3 kD = 1.0 - kS;
		kD *= 1.0 - metallicRoughnessAoShaded.r;

		const vec3 worldNormal = (uInverseView * vec4(N, 0.0)).xyz;
		const vec4 worldPos4 = uInverseView * viewSpacePosition;
		const vec3 worldPos = worldPos4.xyz / worldPos4.w;

#if IRRADIANCE_VOLUME
		const vec3 irradiance = sphericalHarmonicsIrradiance(worldPos, worldNormal);
#else
		const vec3 irradiance = vec3(0.05);
#endif // IRRADIANCE_VOLUME

		const vec3 diffuse = irradiance * albedo;

		// Screenspace Reflections
#if SSR_ENABLED
		vec3 ssPosition = vec3(vTexCoord, texture(uDepthMap, vTexCoord).x);
		const vec3 vsPosition = unproject(ssPosition);
		const vec3 vsDir = normalize(vsPosition);
		const vec3 vsR = reflect(vsDir, N);
		const vec3 ssO = project(vsPosition + vsR * Z_NEAR);
		const vec3 R = ssO - ssPosition;

		const bool hit = linearRayTrace(ssPosition, R);

		const vec2 dCoords = smoothstep(0.2, 0.5, abs(vec2(0.5) - ssPosition.xy));
		const float edgeFactor = clamp(1.0 - (dCoords.x + dCoords.y), 0.0, 1.0);

		// reproject
		vec4 reprojected = uReProjection * vec4(ssPosition * 2.0 - 1.0, 1.0);
		reprojected.xy /= reprojected.w;
		reprojected.xy = reprojected.xy * 0.5 + 0.5;

		const vec3 ssrColor = textureLod(uPrevFrame, reprojected.xy, metallicRoughnessAoShaded.g * log2(textureSize(uPrevFrame, 0).x)).rgb;

		const vec3 correctedTexCoord = parallaxCorrect((uInverseView * vec4(reflect(-V, N), 0.0)).xyz, worldPos);
		const vec3 cubeColor = textureLod(uPrefilterMap, octEncode(correctedTexCoord) * 0.5 + 0.5, metallicRoughnessAoShaded.g * MAX_REFLECTION_LOD).rgb;
		const vec3 prefilteredColor = mix(cubeColor, ssrColor, float(hit) * edgeFactor);
#else
		const vec3 correctedTexCoord = parallaxCorrect((uInverseView * vec4(reflect(-V, N), 0.0)).xyz, worldPos);
		// sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
		const vec3 prefilteredColor = textureLod(uPrefilterMap, octEncode(correctedTexCoord) * 0.5 + 0.5, metallicRoughnessAoShaded.g * MAX_REFLECTION_LOD).rgb;
#endif // SSR_ENABLED

		const vec2 brdf  = texture(uBrdfLUT, vec2(NdotV, metallicRoughnessAoShaded.g)).rg;
		const vec3 specular = prefilteredColor * (kS * brdf.x + brdf.y);

		oFragColor = vec4((kD * diffuse + specular) * metallicRoughnessAoShaded.b, 1.0);
    }
	else
	{
		oFragColor = vec4(albedo, 1.0);
	}
}