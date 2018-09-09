#version 450 core

const int ALBEDO = 1;
const int NORMAL = 2;
const int METALLIC = 4;
const int ROUGHNESS = 8;
const int AO = 16;
const int EMISSIVE = 32;

layout(location = 0) out vec4 oAlbedoRMS;
layout(location = 1) out vec4 oNormalAo;
layout(location = 2) out vec4 oVelocity;
layout(location = 3) out vec4 oEmissive;

layout(early_fragment_tests) in;

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

#include "TBN.h"

vec2 encode (vec3 n)
{
    float f = sqrt(8.0 * n.z + 8.0);
    return n.xy / f + 0.5;
}

vec3 RGB2YCoCg(vec3 c)
{
	return vec3(
	0.25 * c.r + 0.5 * c.g + 0.25 * c.b, 
	0.5 * c.r - 0.5  * c.b + 0.5, 
	-0.25 * c.r + 0.5 * c.g - 0.25 * c.b + 0.5);
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
    if (v.z < 0.0) 
	{
        result = (1.0 - abs(result.yx)) * signNotZero(result.xy);
    }
    return result;
}

vec3 snorm12x2ToUnorm8x3(vec2 f)
{
	vec2 u = vec2(round(clamp(f, -1.0, 1.0) * 2047 + 2047));
	float t = floor(u.y * (1.0 / 256));

	return floor(vec3(u.x * (1.0 / 16.0),
				fract(u.x * (1.0 / 16.0)) * 256.0 + t,
				u.y - t * 256.0)) * (1.0 / 255.0);
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

	vec3 albedoColor = ((uMaterial.mapBitField & ALBEDO) != 0) ? texture(uAlbedoMap, texCoord).rgb : uMaterial.albedo.rgb;
	albedoColor = RGB2YCoCg(albedoColor);

    if((uMaterial.mapBitField & NORMAL) != 0)
    {
        vec3 tangentSpaceNormal = texture(uNormalMap, texCoord).xyz * 2.0 - 1.0;
        N = normalize(TBN * tangentSpaceNormal);
    }
	// we save normals in view space
	N = uViewMatrix * N;
	
	float metallic = ((uMaterial.mapBitField & METALLIC) != 0) ? texture(uMetallicMap, texCoord).r : uMaterial.metallic;
	float roughness = ((uMaterial.mapBitField & ROUGHNESS) != 0) ? texture(uRoughnessMap, texCoord).r : uMaterial.roughness;
	float ao = ((uMaterial.mapBitField & AO) != 0) ? texture(uAoMap, texCoord).r : 1.0;

	ivec2 crd = ivec2(gl_FragCoord.xy);
	const bool pattern = ((crd.x & 1) == (crd.y & 1));
	oAlbedoRMS = vec4(pattern ? albedoColor.rb : albedoColor.rg, roughness, metallic);
	oNormalAo = vec4(snorm12x2ToUnorm8x3(octEncode(N)), ao);

	vec3 emissive = uMaterial.emissive;
	emissive *= ((uMaterial.mapBitField & EMISSIVE) != 0) ? texture(uEmissiveMap, texCoord).rgb : vec3(1.0);

	oEmissive = vec4(emissive, 1.0);

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