#version 450 core

const float PI = 3.14159265359;
const float MAX_REFLECTION_LOD = 4.0;

layout(location = 0) out vec4 oFragColor;

in vec2 vTexCoord;

const int SHADOW_CASCADES = 4;

struct DirectionalLight
{
    vec3 color;
    vec3 direction;
	bool renderShadows;
	mat4 viewProjectionMatrices[SHADOW_CASCADES];
	float splits[SHADOW_CASCADES];
};

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

uniform mat4 uInverseView;
uniform mat4 uProjection;
uniform mat4 uInverseProjection;
uniform mat4 uReProjection;
uniform DirectionalLight uDirectionalLight;
uniform bool uShadowsEnabled;
uniform bool uRenderDirectionalLight;
uniform bool uSsao;
uniform bool uUseSsr;

uniform vec3 uVolumeOrigin;
uniform ivec3 uVolumeDimensions;
uniform float uSpacing;

const float Z_NEAR = 0.1;
const float Z_FAR = 3000.0;

#include "brdf.h"



#define INVALID_HIT_POINT vec3(-1.0)
#define LRM_MAX_ITERATIONS		100						// Maximal number of iterations for linear ray-marching
#define BS_MAX_ITERATIONS		30						// Maximal number of iterations for bineary search
#define BS_DELTA_EPSILON 0.0001

const float Pi = 3.141592654f;
const float CosineA0 = Pi;
const float CosineA1 = (2.0f * Pi) / 3.0f;
const float CosineA2 = Pi * 0.25f;

struct SH9
{
    float c[9];
};

struct SH9Color
{
    vec3 c[9];
};

SH9 SHCosineLobe(in vec3 dir)
{
    SH9 sh;

    // Band 0
    sh.c[0] = 0.282095f * CosineA0;

    // Band 1
    sh.c[1] = 0.488603f * dir.y * CosineA1;
    sh.c[2] = 0.488603f * dir.z * CosineA1;
    sh.c[3] = 0.488603f * dir.x * CosineA1;

    // Band 2
    sh.c[4] = 1.092548f * dir.x * dir.y * CosineA2;
    sh.c[5] = 1.092548f * dir.y * dir.z * CosineA2;
    sh.c[6] = 0.315392f * (3.0f * dir.z * dir.z - 1.0f) * CosineA2;
    sh.c[7] = 1.092548f * dir.x * dir.z * CosineA2;
    sh.c[8] = 0.546274f * (dir.x * dir.x - dir.y * dir.y) * CosineA2;

    return sh;
}

vec3 sphericalHarmonicsIrradiance(vec3 P, vec3 N)
{
	vec3 minCorner = uVolumeOrigin;
	vec3 maxCorner = vec3(uVolumeDimensions - ivec3(1)) * uSpacing.xxx + minCorner;
	P = clamp(P, minCorner, maxCorner);
	
	ivec3 baseGridCoord = ivec3((P - minCorner) / uSpacing);

	float weights[8];
	float totalWeight = 0.0;
	
	for (int i = 0; i < 8; ++i)
	{
		ivec3 offset = ivec3(i, i >> 1, i >> 2) & ivec3(1);
		ivec3 probeCoord = baseGridCoord + offset;
		vec3 probePos = uSpacing * vec3(probeCoord) + minCorner;
		vec3 toProbe = probePos - P;
		vec3 alpha = 1.0 - (abs(toProbe) / uSpacing);
		weights[i] = alpha.x * alpha.y * alpha.z * float(probeCoord == clamp(probeCoord, ivec3(0), uVolumeDimensions - ivec3(1))) * max(0.005, dot(normalize(toProbe), N));
		totalWeight += weights[i];
	}
	
	SH9Color coeffs;
	for (int i = 0; i < 9; ++i)
	{
		coeffs.c[i] = vec3(0.0);
	}
	
	ivec3 dims = uVolumeDimensions;

	for (int i = 0; i < 8; ++i)
	{
		ivec3 coord  = baseGridCoord + (ivec3(i, i >> 1, i >> 2) & ivec3(1));
		int index = coord.z * (dims.x * dims.y) + coord.y * dims.x + coord.x;

		for (int j = 0; j < 9; ++j)
		{
			coeffs.c[j] += weights[i] * texelFetch(uIrradianceMap, ivec2(j, index), 0).rgb;
		}
	}

	 // Compute the cosine lobe in SH, oriented about the normal direction
    SH9 shCosine = SHCosineLobe(N);

    // Compute the SH dot product to get irradiance
    vec3 irradiance = vec3(0.0);
	float rcpTotalWeight = 1.0 / totalWeight;
    for(int i = 0; i < 9; ++i)
    {
		irradiance += coeffs.c[i] * rcpTotalWeight * shCosine.c[i];
	}

    return irradiance;
}

vec3 parallaxCorrect(vec3 R, vec3 P)
{
	const vec3 boxMin = vec3(-9.5, -0.01, -2.4);
	const vec3 boxMax = vec3(9.5, 15.0, 2.4);
	const vec3 probePos = vec3(0.0, 2.0, 0.0);
	
	vec3 firstPlaneIntersect = (boxMax - P) / R;
	vec3 secondPlaneIntersect = (boxMin - P) / R;
	
	vec3 furthestPlane = max(firstPlaneIntersect, secondPlaneIntersect);

	float dist = min(min(furthestPlane.x, furthestPlane.y), furthestPlane.z);
	
	vec3 intersectPos = P + R * dist;

	return (P.x >= boxMin.x && P.y >= boxMin.y && P.z >= boxMin.z && P.x <= boxMax.x && P.y <= boxMax.y && P.z <= boxMax.z) ? normalize(intersectPos - probePos) : R;
}

float linearDepth(float depth)
{
    float z_n = 2.0 * depth - 1.0;
    return 2.0 * Z_NEAR * Z_FAR / (Z_FAR + Z_NEAR - z_n * (Z_FAR - Z_NEAR));
}

vec2 signNotZero(vec2 v) 
{
	return vec2((v.x >= 0.0) ? +1.0 : -1.0, (v.y >= 0.0) ? +1.0 : -1.0);
}

/** Assumes that v is a unit vector. The result is an octahedral vector on the [-1, +1] square. */
vec2 octEncode(in vec3 v) 
{
    float l1norm = abs(v.x) + abs(v.y) + abs(v.z);
    vec2 result = v.xy * (1.0 / l1norm);
    if (v.z < 0.0) {
        result = (1.0 - abs(result.yx)) * signNotZero(result.xy);
    }
    return result;
}

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
	vec4 vsCoord = uInverseProjection * vec4(ssCoord, 1.0);
	return vsCoord.xyz / vsCoord.w;
}

bool linearRayTraceBinarySearch(inout vec3 rayPos, vec3 rayDir)
{
	for (uint i = 0; i < BS_MAX_ITERATIONS; ++i)
	{		
		/* Check if we found our final hit point */
		float depth = textureLod(uDepthMap, rayPos.xy, 0.0).r;
		float depthDelta = linearDepth(depth) - linearDepth(rayPos.z);
		
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
	vec3 rayStart = rayPos;
	vec3 rayEnd = rayStart + rayDir;
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

void main()
{
	vec2 texCoord = gl_FragCoord.xy / textureSize(uAlbedoMap, 0);

	vec3 albedo = texture(uAlbedoMap, texCoord).rgb;
	
	vec4 metallicRoughnessAoShaded = texture(uMetallicRoughnessAoMap, texCoord).rgba;

	if(uSsao)
	{
		metallicRoughnessAoShaded.b = min(metallicRoughnessAoShaded.b, texture(uSsaoMap, texCoord).r);
	}
    
		
    if (metallicRoughnessAoShaded.a > 0.0)
    {
		vec3 N = texture(uNormalMap, texCoord).xyz;//decode(texture(uNormalMap, texCoord).xy);
		
		float depth = texture(uDepthMap, texCoord).r;
		vec4 clipSpacePosition = vec4(vTexCoord * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
		vec4 viewSpacePosition = uInverseProjection * clipSpacePosition;
		viewSpacePosition /= viewSpacePosition.w;

		vec3 F0 = mix(vec3(0.04), albedo, metallicRoughnessAoShaded.r);
		vec3 V = -normalize(viewSpacePosition.xyz);
		float NdotV = max(dot(N, V), 0.0);

		vec3 directionalLightContribution = vec3(0.0);

		if(uRenderDirectionalLight)
		{
			// shadow
			float shadow = 0.0;
			if(uDirectionalLight.renderShadows && uShadowsEnabled)
			{		
				float split = SHADOW_CASCADES - 1.0;
				for (float i = 0.0; i < SHADOW_CASCADES; ++i)
				{
					if(-viewSpacePosition.z < uDirectionalLight.splits[int(i)])
					{
						split = i;
						break;
					}
				}

				vec4 worldPos4 = uInverseView * viewSpacePosition;
				worldPos4 /= worldPos4.w;
				vec4 projCoords4 = uDirectionalLight.viewProjectionMatrices[int(split)] * worldPos4;
				vec3 projCoords = (projCoords4 / projCoords4.w).xyz;
				projCoords = projCoords * 0.5 + 0.5; 
				vec2 invShadowMapSize = vec2(1.0 / (textureSize(uShadowMap, 0).xy));

				float count = 0.0;
				float radius = 2.0;
				for(float row = -radius; row <= radius; ++row)
				{
					for(float col = -radius; col <= radius; ++col)
					{
						++count;
						shadow += texture(uShadowMap, vec4(projCoords.xy + vec2(col, row) * invShadowMapSize, split, projCoords.z - 0.001)).x;
					}
				}
				shadow *= 1.0 / count;
			}

			vec3 L = normalize(uDirectionalLight.direction);
			vec3 H = normalize(V + L);
			float NdotL = max(dot(N, L), 0.0);

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
				
			directionalLightContribution = vec3((kD * albedo.rgb / PI + specular) * uDirectionalLight.color * NdotL * (1.0 - shadow));
		}
		
		vec3 ambientLightContribution = vec3(0.0);

		if (metallicRoughnessAoShaded.a > 0.125)
		{
			// ambient lighting using IBL
			vec3 kS = fresnelSchlickRoughness(NdotV, F0, metallicRoughnessAoShaded.g);
			vec3 kD = 1.0 - kS;
			kD *= 1.0 - metallicRoughnessAoShaded.r;

			vec3 worldNormal = (uInverseView * vec4(N, 0.0)).xyz;
			vec4 worldPos4 = uInverseView * viewSpacePosition;

			vec3 irradiance = sphericalHarmonicsIrradiance(worldPos4.xyz / worldPos4.w, worldNormal);//vec3(0.05);//textureLod(uIrradianceMap, octEncode(worldNormal) * 0.5 + 0.5, 0.0).rgb;
			vec3 diffuse = irradiance * albedo;

			vec3 prefilteredColor = vec3(0.0);

			// Screenspace Reflections
			if(uUseSsr)
			{
				vec3 ssPosition = vec3(vTexCoord, texture(uDepthMap, vTexCoord).x);
				vec3 vsPosition = unproject(ssPosition);
				vec3 vsDir = normalize(vsPosition);
				vec3 vsR = reflect(vsDir, N);
				vec3 ssO = project(vsPosition + vsR * Z_NEAR);
				vec3 R = ssO - ssPosition;

				bool hit = linearRayTrace(ssPosition, R);

				vec2 dCoords = smoothstep(0.2, 0.5, abs(vec2(0.5) - ssPosition.xy));
				float edgeFactor = clamp(1.0 - (dCoords.x + dCoords.y), 0.0, 1.0);

				// reproject
				vec4 reprojected = uReProjection * vec4(ssPosition * 2.0 - 1.0, 1.0);
				reprojected.xy /= reprojected.w;
				reprojected.xy = reprojected.xy * 0.5 + 0.5;

				vec3 ssrColor = textureLod(uPrevFrame, reprojected.xy, metallicRoughnessAoShaded.g * log2(textureSize(uPrevFrame, 0).x)).rgb;

				vec4 worldPos4 = uInverseView * viewSpacePosition;
				vec3 correctedTexCoord = parallaxCorrect((uInverseView * vec4(reflect(-V, N), 0.0)).xyz, worldPos4.xyz / worldPos4.w);
				vec3 cubeColor = textureLod(uPrefilterMap, octEncode(correctedTexCoord) * 0.5 + 0.5, metallicRoughnessAoShaded.g * MAX_REFLECTION_LOD).rgb;
				prefilteredColor = mix(cubeColor, ssrColor, float(hit) * edgeFactor);

			}
			else
			{
				vec4 worldPos4 = uInverseView * viewSpacePosition;
				vec3 correctedTexCoord = parallaxCorrect((uInverseView * vec4(reflect(-V, N), 0.0)).xyz, worldPos4.xyz / worldPos4.w);
				// sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
				prefilteredColor = textureLod(uPrefilterMap, octEncode(correctedTexCoord) * 0.5 + 0.5, metallicRoughnessAoShaded.g * MAX_REFLECTION_LOD).rgb;
			}

			vec2 brdf  = texture(uBrdfLUT, vec2(NdotV, metallicRoughnessAoShaded.g)).rg;
			vec3 specular = prefilteredColor * (kS * brdf.x + brdf.y);

			ambientLightContribution = metallicRoughnessAoShaded.a * (kD * diffuse + specular) * metallicRoughnessAoShaded.b;
		}

		oFragColor = vec4(ambientLightContribution + directionalLightContribution, 1.0);
    }
	else
	{
		oFragColor = vec4(albedo, 1.0);
	}
}