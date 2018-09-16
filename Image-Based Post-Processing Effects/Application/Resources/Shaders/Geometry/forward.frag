#version 450 core

const float PI = 3.14159265359;
const int ALBEDO = 1;
const int NORMAL = 2;
const int METALLIC = 4;
const int ROUGHNESS = 8;
const int AO = 16;
const int EMISSIVE = 32;

layout(location = 0) out vec4 oFragColor;
layout(location = 1) out vec4 oVelocity;

layout(early_fragment_tests) in;

in vec2 vTexCoord;
in vec3 vNormal;
in vec4 vCurrentPos;
in vec4 vPrevPos;
in vec3 vWorldPos;

const int SHADOW_CASCADES = 3;

struct Material
{
    vec4 albedo;
    float metallic;
    float roughness;
	vec3 emissive;
    int mapBitField;
	bool displacement;
};

struct DirectionalLight
{
    vec3 color;
    vec3 direction;
	bool renderShadows;
	mat4 viewProjectionMatrices[SHADOW_CASCADES];
	float splits[SHADOW_CASCADES];
};

layout(binding = 0) uniform sampler2D albedoMap;
layout(binding = 1) uniform sampler2D normalMap;
layout(binding = 2) uniform sampler2D metallicMap;
layout(binding = 3) uniform sampler2D roughnessMap;
layout(binding = 4) uniform sampler2D aoMap;
layout(binding = 5) uniform sampler2D emissiveMap;
layout(binding = 6) uniform sampler2D uDisplacementMap;
layout(binding = 15) uniform sampler2DArrayShadow uShadowMap;
layout(binding = 12) uniform sampler2D uIrradianceMap;
layout(binding = 13) uniform sampler2D uPrefilterMap;
layout(binding = 9) uniform sampler2D uBrdfLUT;

uniform Material uMaterial;
uniform DirectionalLight uDirectionalLight;
uniform bool uRenderDirectionalLight;
uniform vec3 uCamPos;
uniform bool uShadowsEnabled;
uniform mat4 uViewMatrix;

#include "brdf.h"
#include "TBN.h"

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

void main()
{
	vec2 texCoord = vTexCoord;

	vec3 N = normalize(vNormal);
	mat3 TBN = calculateTBN(N, vWorldPos, texCoord);

	if(uMaterial.displacement)
	{
		const float heightScale = 0.05;
	
		const float minLayers = 8.0;
		const float maxLayers = 32.0;

		vec3 viewDir = normalize(uCamPos - vWorldPos) * TBN;
		float numLayers = mix(maxLayers, minLayers, abs(viewDir.z));

		// the amount to shift the texture coordinates per layer (from vector P)
		vec2 P = viewDir.xy * heightScale; 

		float layerDepth = 1.0 / numLayers;
		vec2 deltaTexCoord = P / numLayers;
	
		// get initial values
		vec2  currentTexCoord  = texCoord;
		float currentDepthMapValue = texture(uDisplacementMap, texCoord).r;
		float previousDepthMapValue = 0.0;
		float currentLayerDepth = 0.0;
		  
		while(currentLayerDepth < currentDepthMapValue)
		{
		    currentTexCoord -= deltaTexCoord;
	
			previousDepthMapValue = currentDepthMapValue;
		    currentDepthMapValue = texture(uDisplacementMap, currentTexCoord).r;  
	
		    currentLayerDepth += layerDepth;  
		}
		
		// get depth after and before collision for linear interpolation
		float afterDepth  = currentDepthMapValue - currentLayerDepth;
		float beforeDepth = previousDepthMapValue - currentLayerDepth + layerDepth;
		 
		// interpolation of texture coordinates
		float weight = afterDepth / (afterDepth - beforeDepth);
		texCoord = mix(currentTexCoord, currentTexCoord + deltaTexCoord, weight);
	}

    vec4 albedo;
    if((uMaterial.mapBitField & ALBEDO) == ALBEDO)
    {
		albedo = texture(albedoMap, texCoord).rgba; 
        albedo.rgb = pow(albedo.rgb, vec3(2.2));
    }
    else
    {
        albedo = uMaterial.albedo;
    }

	// TODO: switch to view space normals for uniformity
    if((uMaterial.mapBitField & NORMAL) == NORMAL)
    {
        vec3 tangentSpaceNormal = texture(normalMap, texCoord).xyz * 2.0 - 1.0;
        N = normalize(TBN * tangentSpaceNormal);
    }

    float metallic;
    if((uMaterial.mapBitField & METALLIC) == METALLIC)
    {
        metallic  = texture(metallicMap, texCoord).r;
    }
    else
    {
        metallic = uMaterial.metallic;
    }

    float roughness;
    if((uMaterial.mapBitField & ROUGHNESS) == ROUGHNESS)
    {
        roughness = texture(roughnessMap, texCoord).r;
    }
    else
    {
        roughness = uMaterial.roughness;
    }

    float ao;
    if((uMaterial.mapBitField & AO) == AO)
    {
        ao = texture(aoMap, texCoord).r;
    }
    else
    {
        ao = 1.0;
    }

	vec3 emissive;
	if((uMaterial.mapBitField & EMISSIVE) != 0)
    {
        emissive = uMaterial.emissive * texture(emissiveMap, texCoord).rgb;
    }
    else
    {
        emissive = uMaterial.emissive;
    }
	
	vec3 V = normalize(uCamPos - vWorldPos.xyz);
	float NdotV = max(dot(N, V), 0.0);
	vec3 F0 = mix(vec3(0.04), albedo.rgb, metallic);
	
	vec3 directionalLightContribution = vec3(0.0);

	if (uRenderDirectionalLight)
	{
		// shadow
		float shadow = 0.0;
		if(uDirectionalLight.renderShadows && uShadowsEnabled)
		{		
			vec4 viewSpacePosition = uViewMatrix * vec4(vWorldPos, 1.0);
			viewSpacePosition.xyz /= viewSpacePosition.w;

			float split = SHADOW_CASCADES - 1.0;
			for (float i = 0.0; i < SHADOW_CASCADES; ++i)
			{
				if(-viewSpacePosition.z < uDirectionalLight.splits[int(i)])
				{
					split = i;
					break;
				}
			}

			vec4 projCoords4 = uDirectionalLight.viewProjectionMatrices[int(split)] * vec4(vWorldPos, 1.0);
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
		float NDF = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(NdotV, NdotL, roughness);
		vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

		vec3 nominator = NDF * G * F;
		float denominator = max(4 * NdotV * NdotL, 0.0000001);

		vec3 specular = nominator / denominator;

		// because of energy conversion kD and kS must add up to 1.0
		vec3 kD = vec3(1.0) - F;
		// multiply kD by the inverse metalness so if a material is metallic, it has no diffuse lighting (and otherwise a blend)
		kD *= 1.0 - metallic;
				
		directionalLightContribution = vec3((kD * albedo.rgb / PI + specular) * uDirectionalLight.color * NdotL * (1.0 - shadow));
	}
				
	// ambient lighting using IBL
	vec3 kS = fresnelSchlickRoughness(NdotV, F0,roughness);
	vec3 kD = 1.0 - kS;
	kD *= 1.0 - metallic;

	vec3 irradiance = vec3(0.05);//texture(uIrradianceMap, N).rgb;
	vec3 diffuse = irradiance * albedo.rgb;

	// sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
	const float MAX_REFLECTION_LOD = 4.0;
	vec3 correctedTexCoord = parallaxCorrect(reflect(-V, N), vWorldPos);
	vec3 prefilteredColor = textureLod(uPrefilterMap, octEncode(correctedTexCoord) * 0.5 + 0.5, roughness * MAX_REFLECTION_LOD).rgb;
	vec2 brdf  = texture(uBrdfLUT, vec2(NdotV, roughness)).rg;
	vec3 specular = prefilteredColor * (kS * brdf.x + brdf.y);
				
	vec3 ambientLightContribution = vec3((kD * diffuse + specular) * ao);

	oFragColor = vec4(directionalLightContribution + ambientLightContribution + emissive, albedo.a);

	vec2 a = (vCurrentPos.xy / vCurrentPos.w) * 0.5 + 0.5;
    vec2 b = (vPrevPos.xy / vPrevPos.w) * 0.5 + 0.5;
	vec2 v = a - b;
	//v = pow(v, vec2(3.0));
	oVelocity = vec4(v, 0.0, 0.0); // vec4(a - b, 0.0, 0.0); //vec4(pow((a - b) * 0.5 + 0.5, vec2(3.0)), 0.0, 0.0);
}