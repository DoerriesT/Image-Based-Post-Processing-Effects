#version 450 core

#ifndef DIRECTIONAL_LIGHT_ENABLED
#define DIRECTIONAL_LIGHT_ENABLED 0
#endif // DIRECTIONAL_LIGHT_ENABLED

#ifndef SHADOWS_ENABLED
#define SHADOWS_ENABLED 0
#endif // SHADOWS_ENABLED

#ifndef SSAO_ENABLED
#define SSAO_ENABLED 0
#endif // SSAO_ENABLED

#ifndef GTAO_MULTI_BOUNCE_ENABLED
#define GTAO_MULTI_BOUNCE_ENABLED 0
#endif // GTAO_MULTI_BOUNCE_ENABLED

#ifndef IRRADIANCE_SOURCE
#define IRRADIANCE_SOURCE 0
#endif // IRRADIANCE_SOURCE

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
layout(binding = 15) uniform sampler2DArrayShadow uShadowMap;
layout(binding = 5) uniform sampler2D uRedVolume;
layout(binding = 6) uniform sampler2D uGreenVolume;
layout(binding = 7) uniform sampler2D uBlueVolume;

#if DIRECTIONAL_LIGHT_ENABLED
const int SHADOW_CASCADES = 3;

struct DirectionalLight
{
    vec3 color;
    vec3 direction;
	bool renderShadows;
	mat4 viewProjectionMatrices[SHADOW_CASCADES];
	float splits[SHADOW_CASCADES];
};

uniform DirectionalLight uDirectionalLight;
#endif // DIRECTIONAL_LIGHT_ENABLED

uniform bool uOddFrame;
uniform mat4 uInverseView;
uniform mat4 uInverseProjection;

#if IRRADIANCE_SOURCE
uniform vec3 uVolumeOrigin;
uniform ivec3 uVolumeDimensions;
uniform float uSpacing;
#endif // IRRADIANCE_SOURCE

const float PI = 3.14159265359;
const float MAX_REFLECTION_LOD = 4.0;
const float Z_NEAR = 0.1;
const float Z_FAR = 300.0;

#include "brdf.h"

float interleavedGradientNoise(vec2 v)
{
	vec3 magic = vec3(0.06711056, 0.00583715, 52.9829189);
	return fract(magic.z * dot(v, magic.xy));
}

#if IRRADIANCE_SOURCE == 1 // precomputed irradiance volume

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
		const float weight = alpha.x * alpha.y * alpha.z * float(probeCoord == clamp(probeCoord, ivec3(0), uVolumeDimensions - ivec3(1)));// * max(0.005, dot(normalize(toProbe), N));
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

    return max(irradiance, vec3(0.0)) * (1.0 / totalWeight);
}

#elif IRRADIANCE_SOURCE == 2 // light propagation volume

/*Spherical harmonics coefficients ? precomputed*/
#define SH_C0 0.282094792f // 1 / 2sqrt(pi)
#define SH_C1 0.488602512f // sqrt(3/pi) / 2

vec4 dirToSH(vec3 dir) 
{
	return vec4(SH_C0, -SH_C1 * dir.y, SH_C1 * dir.z, -SH_C1 * dir.x);
}

vec3 getGridPos(vec3 position)
{
	return (position - uVolumeOrigin) / uSpacing;
}

#define SAMPLE_TRILINEAR(sampler, texCoords, alpha) mix(texture(sampler, texCoords.xy), texture(sampler, texCoords.zw), alpha)

vec3 lpvIrradiance(vec3 P, vec3 N)
{
	vec4 shIntensity = dirToSH(-N);
	vec3 gridCell = getGridPos(P);

	float zFloor = floor(gridCell.z);

	vec4 texCoords;
	texCoords.xy = vec2(gridCell.x / (uVolumeDimensions.x * uVolumeDimensions.z) + zFloor / uVolumeDimensions.z , gridCell.y / uVolumeDimensions.y);
	texCoords.zw = vec2(texCoords.x + (1.0 / uVolumeDimensions.x), texCoords.y);
	float alpha = gridCell.z - zFloor;

	vec4 red = SAMPLE_TRILINEAR(uRedVolume, texCoords, alpha);
	vec4 green = SAMPLE_TRILINEAR(uGreenVolume, texCoords, alpha);
	vec4 blue = SAMPLE_TRILINEAR(uBlueVolume, texCoords, alpha);
	return max(vec3(dot(shIntensity, red), dot(shIntensity, green), dot(shIntensity, blue)), vec3(0.0)) / PI;
}

#endif // IRRADIANCE_SOURCE

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

vec3 GTAOMultiBounce(float visibility, vec3 albedo)
{
	vec3 a = 2.0404 * albedo - 0.3324;
	vec3 b = -4.7951 * albedo + 0.6417;
	vec3 c = 2.7552 * albedo + 0.6903;

	float x = visibility;
	return max(x.xxx, ((x * a + b) * x + c) * x);
}

void main()
{
	oFragColor = vec4(0.0, 0.0, 0.0, 1.0);

	const vec2 texCoord = gl_FragCoord.xy / textureSize(uAlbedoMap, 0);

	const vec3 albedo = texture(uAlbedoMap, texCoord).rgb;
	
	vec4 metallicRoughnessAoShaded = texture(uMetallicRoughnessAoMap, texCoord).rgba;

	if (metallicRoughnessAoShaded.a == 0.0)
	{
		discard;
	}

#if SSAO_ENABLED
	metallicRoughnessAoShaded.b = min(metallicRoughnessAoShaded.b, texture(uSsaoMap, texCoord).r);
#endif // SSAO_ENABLED
    
	const float depth = texture(uDepthMap, texCoord).r;
	const vec4 clipSpacePosition = vec4(vTexCoord * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
	vec4 viewSpacePosition = uInverseProjection * clipSpacePosition;
	viewSpacePosition /= viewSpacePosition.w;

	const vec3 F0 = mix(vec3(0.04), albedo, metallicRoughnessAoShaded.r);
	const vec3 V = -normalize(viewSpacePosition.xyz);
	const vec3 N = texture(uNormalMap, texCoord).xyz;//decode(texture(uNormalMap, texCoord).xy);
	const float NdotV = max(dot(N, V), 0.0);

#if DIRECTIONAL_LIGHT_ENABLED
	{
		const vec3 L = uDirectionalLight.direction;
		const vec3 H = normalize(V + L);
		const float NdotL = max(dot(N, L), 0.0);
		
		// Cook-Torrance BRDF
		const float NDF = DistributionGGX(N, H, metallicRoughnessAoShaded.g);
		const float G = GeometrySmith(NdotV, NdotL, metallicRoughnessAoShaded.g);
		const vec3 F0 = mix(vec3(0.04), albedo, metallicRoughnessAoShaded.r);
		const vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

		const vec3 nominator = NDF * G * F;
		const float denominator = max(4 * NdotV * NdotL, 0.0000001);

		const vec3 specular = nominator / denominator;

		// because of energy conversion kD and kS must add up to 1.0
		vec3 kD = vec3(1.0) - F;
		// multiply kD by the inverse metalness so if a material is metallic, it has no diffuse lighting (and otherwise a blend)
		kD *= 1.0 - metallicRoughnessAoShaded.r;

		oFragColor.rgb += (kD * albedo.rgb / PI + specular) * uDirectionalLight.color * NdotL;
	}
#endif // DIRECTIONAL_LIGHT_ENABLED

	const vec4 worldPos4 = uInverseView * viewSpacePosition;

#if SHADOWS_ENABLED && DIRECTIONAL_LIGHT_ENABLED
	if(uDirectionalLight.renderShadows)
	{		
		float split = SHADOW_CASCADES - 1.0;
		vec3 shadowCoord = vec3(2.0);
		for (float i = 0.0; i < SHADOW_CASCADES; ++i)
		{
			const vec4 projCoords4 = uDirectionalLight.viewProjectionMatrices[int(i)] * uInverseView * vec4(0.1 * uDirectionalLight.direction + viewSpacePosition.xyz, 1.0);
			shadowCoord = (projCoords4 / projCoords4.w).xyz * 0.5 + 0.5; 

			// test if projected coordinate is inside texture
			// add small guard band at edges to avoid PCF sampling outside texture
			if(all(greaterThanEqual(shadowCoord.xy, vec2(0.003))) && all(lessThan(shadowCoord.xy, vec2(1.0 - 0.003))))
			{
				split = i;
				break;
			}
		}

		const vec2 invShadowMapSize = vec2(1.0 / (textureSize(uShadowMap, 0).xy));

		float shadow = 0.0;

		const float noise = interleavedGradientNoise(gl_FragCoord.xy);// * 0.5 + (uOddFrame ? 0.5 : 0.0);

		const float rotSin = sin(2.0 * PI * noise);
		const float rotCos = cos(2.0 * PI * noise);

		const mat2 rotation = mat2(rotCos, rotSin, -rotSin, rotCos);

		const vec2 samples[8] = 
		{ 
			vec2(-0.7071, 0.7071),
			vec2(0.0, -0.8750),
			vec2(0.5303, 0.5303),
			vec2(-0.625, 0.0),
			vec2(0.3536, -0.3536),
			vec2(0.0, 0.375),
			vec2(-0.1768, -0.1768),
			vec2(0.125, 0.0)
		};

		const float splitMult = split == 0.0 ? 5.0 : split == 1.0 ? 2.0 : 1.0;

		for(int i = 0; i < 8; ++i)
		{
			vec2 offset = rotation * samples[i];
			shadow += texture(uShadowMap, vec4(shadowCoord.xy + offset * invShadowMapSize * splitMult, split, shadowCoord.z)).x;
		}
		shadow *= 1.0 / 8.0;

		// assuming there is only the directional light contribution in oFragColor.rgb
		oFragColor.rgb *= (1.0 - shadow);
	}	
#endif // SHADOWS_ENABLED && DIRECTIONAL_LIGHT_ENABLED
	
	// ambient lighting using IBL
	const vec3 kS = fresnelSchlickRoughness(NdotV, F0, metallicRoughnessAoShaded.g);
	vec3 kD = 1.0 - kS;
	kD *= 1.0 - metallicRoughnessAoShaded.r;

	const vec3 worldPos = worldPos4.xyz / worldPos4.w;

#if IRRADIANCE_SOURCE == 1
	const vec3 worldNormal = (uInverseView * vec4(N, 0.0)).xyz;
	const vec3 irradiance = sphericalHarmonicsIrradiance(worldPos, worldNormal);
#elif IRRADIANCE_SOURCE == 2
	const vec3 worldNormal = (uInverseView * vec4(N, 0.0)).xyz;
	const vec3 irradiance = lpvIrradiance(worldPos, worldNormal);
#else
	const vec3 irradiance = vec3(0.05);
#endif // IRRADIANCE_SOURCE

#if GTAO_MULTI_BOUNCE_ENABLED
	const vec3 diffuse = irradiance * albedo * GTAOMultiBounce(metallicRoughnessAoShaded.z, albedo);
#else
	const vec3 diffuse = irradiance * albedo * metallicRoughnessAoShaded.z;
#endif // GTAO_MULTI_BOUNCE_ENABLED

	const vec3 correctedTexCoord = parallaxCorrect((uInverseView * vec4(reflect(-V, N), 0.0)).xyz, worldPos);
	// sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
	const vec3 prefilteredColor = textureLod(uPrefilterMap, octEncode(correctedTexCoord) * 0.5 + 0.5, metallicRoughnessAoShaded.g * MAX_REFLECTION_LOD).rgb;

	const vec2 brdf  = texture(uBrdfLUT, vec2(NdotV, metallicRoughnessAoShaded.g)).rg;
	const vec3 specular = prefilteredColor * (kS * brdf.x + brdf.y);

	oFragColor.rgb += (kD * diffuse);
}