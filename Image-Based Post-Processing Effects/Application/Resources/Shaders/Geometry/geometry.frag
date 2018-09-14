#version 450 core

const int ALBEDO = 1;
const int NORMAL = 2;
const int METALLIC = 4;
const int ROUGHNESS = 8;
const int AO = 16;
const int EMISSIVE = 32;

layout(location = 0) out vec4 oAlbedo;
layout(location = 1) out vec4 oNormal;
layout(location = 2) out vec4 oMetallicRoughnessAo;
layout(location = 3) out vec4 oVelocity;
layout(location = 4) out vec4 oEmissive;

//layout(early_fragment_tests) in;

in vec2 vTexCoord;
in vec3 vNormal;
in vec4 vCurrentPos;
in vec4 vPrevPos;
in vec3 vWorldPos;

struct Material
{
    vec4 albedo;
    float metallic;
    float roughness;
	vec3 emissive;
    int mapBitField;
	bool displacement;
};

layout(binding = 0) uniform sampler2D uAlbedoMap;
layout(binding = 1) uniform sampler2D uNormalMap;
layout(binding = 2) uniform sampler2D uMetallicMap;
layout(binding = 3) uniform sampler2D uRoughnessMap;
layout(binding = 4) uniform sampler2D uAoMap;
layout(binding = 5) uniform sampler2D uEmissiveMap;
layout(binding = 6) uniform sampler2D uDisplacementMap;

uniform Material uMaterial;
uniform mat3 uViewMatrix;
uniform float uExposureTime = 0.5;
uniform float uMaxVelocityMag;
uniform vec2 uVel;
uniform vec3 uCamPos;

const float MAX_VELOCITY = 25 * 0.000625; // tilesize * pixel width
const float ALPHA_CUTOFF = 0.9;
const float MIP_SCALE = 0.25;

#include "TBN.h"

vec2 encode (vec3 n)
{
    float f = sqrt(8.0 * n.z + 8.0);
    return n.xy / f + 0.5;
}

vec3 accurateLinearToSRGB(in vec3 linearCol)
{
	vec3 sRGBLo = linearCol * 12.92;
	vec3 sRGBHi = (pow(abs(linearCol), vec3(1.0/2.4)) * 1.055) - 0.055;
	vec3 sRGB = mix(sRGBLo, sRGBHi, vec3(greaterThan(linearCol, vec3(0.0031308))));
	return sRGB;
}

void main()
{
	vec2 texCoord = vTexCoord;

	vec3 N = normalize(vNormal);
	mat3 TBN = calculateTBN( N, vWorldPos, texCoord);

	if(uMaterial.displacement)
	{
		const float heightScale = 0.025;
	
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

    if((uMaterial.mapBitField & ALBEDO) != 0)
    {
		vec4 albedo = texture(uAlbedoMap, texCoord).rgba;
		albedo.a *= 1.0 + textureQueryLod(uAlbedoMap, texCoord).x * MIP_SCALE;
		if(albedo.a < ALPHA_CUTOFF)
		{
			discard;
		}
		oAlbedo = vec4(albedo.rgb, 1.0);
    }
	else
	{
		oAlbedo = vec4(accurateLinearToSRGB(uMaterial.albedo.xyz), 1.0);
	}

    if((uMaterial.mapBitField & NORMAL) != 0)
    {
        vec3 tangentSpaceNormal = texture(uNormalMap, texCoord).xyz * 2.0 - 1.0;
        N = normalize(TBN * tangentSpaceNormal);
    }
	// we save normals in view space
	oNormal = vec4(uViewMatrix * N, 0.0);

    if((uMaterial.mapBitField & METALLIC) != 0)
    {
        oMetallicRoughnessAo.r  = texture(uMetallicMap, texCoord).r;
    }
    else
    {
        oMetallicRoughnessAo.r = uMaterial.metallic;
    }

    if((uMaterial.mapBitField & ROUGHNESS) != 0)
    {
        oMetallicRoughnessAo.g = texture(uRoughnessMap, texCoord).r;
    }
    else
    {
        oMetallicRoughnessAo.g = uMaterial.roughness;
    }

    if((uMaterial.mapBitField & AO) != 0)
    {
        oMetallicRoughnessAo.b = texture(uAoMap, texCoord).r;
    }
    else
    {
        oMetallicRoughnessAo.b = 1.0;
    }

	if((uMaterial.mapBitField & EMISSIVE) != 0)
    {
        oEmissive = vec4(uMaterial.emissive * texture(uEmissiveMap, texCoord).rgb, 1.0);
    }
    else
    {
        oEmissive = vec4(uMaterial.emissive, 0.0);
    }

	oMetallicRoughnessAo.a = 1.0;

	vec2 a = (vCurrentPos.xy / vCurrentPos.w) * 0.5 + 0.5;
    vec2 b = (vPrevPos.xy / vPrevPos.w) * 0.5 + 0.5;
	vec2 v = a - b;

	//v *= uExposureTime;
	//v = uVel;

	// clamp to maximum length
	//float originalLength = length(v);
	//v *= min(uMaxVelocityMag / originalLength, 1.0);

	oVelocity = vec4(v, 0.0, 0.0);
}