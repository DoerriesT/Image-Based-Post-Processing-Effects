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

in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
in vec4 vCurrentPos;
in vec4 vPrevPos;

struct Material
{
    vec4 albedo;
    float metallic;
    float roughness;
	vec3 emissive;
    int mapBitField;
};

layout(binding = 0) uniform sampler2D uAlbedoMap;
layout(binding = 1) uniform sampler2D uNormalMap;
layout(binding = 2) uniform sampler2D uMetallicMap;
layout(binding = 3) uniform sampler2D uRoughnessMap;
layout(binding = 4) uniform sampler2D uAoMap;
layout(binding = 5) uniform sampler2D uEmissiveMap;

uniform Material uMaterial;
uniform float uExposureTime = 0.5;
uniform float uMaxVelocityMag;
uniform vec2 uVel;

const float MAX_VELOCITY = 25 * 0.000625; // tilesize * pixel width

vec2 encode (vec3 n)
{
    float f = sqrt(8.0 * n.z + 8.0);
    return n.xy / f + 0.5;
}

void main()
{
    if((uMaterial.mapBitField & ALBEDO) != 0)
    {
		oAlbedo = vec4(texture(uAlbedoMap, vTexCoord).rgb, 1.0);
    }
	else
	{
		oAlbedo = uMaterial.albedo;
	}

	// we save normals in view space
    vec3 N = normalize(vNormal);
    if((uMaterial.mapBitField & NORMAL) != 0)
    {
        vec3 tangentNormal = texture(uNormalMap, vTexCoord).xyz * 2.0 - 1.0;
        N = normalize(mat3(normalize(vTangent), -normalize(vBitangent), N) * tangentNormal);
    }
	oNormal = vec4(encode(N), 0.0, 0.0);
	

    if((uMaterial.mapBitField & METALLIC) != 0)
    {
        oMetallicRoughnessAo.r  = texture(uMetallicMap, vTexCoord).r;
    }
    else
    {
        oMetallicRoughnessAo.r = uMaterial.metallic;
    }

    if((uMaterial.mapBitField & ROUGHNESS) != 0)
    {
        oMetallicRoughnessAo.g = texture(uRoughnessMap, vTexCoord).r;
    }
    else
    {
        oMetallicRoughnessAo.g = uMaterial.roughness;
    }

    if((uMaterial.mapBitField & AO) != 0)
    {
        oMetallicRoughnessAo.b = texture(uAoMap, vTexCoord).r;
    }
    else
    {
        oMetallicRoughnessAo.b = 1.0;
    }

	if((uMaterial.mapBitField & EMISSIVE) != 0)
    {
        oEmissive = vec4(uMaterial.emissive * texture(uEmissiveMap, vTexCoord).rgb, 1.0);
    }
    else
    {
        oEmissive = vec4(uMaterial.emissive, 0.0);
    }

	oMetallicRoughnessAo.a = 1.0;

	vec2 a = (vCurrentPos.xy / vCurrentPos.w);
    vec2 b = (vPrevPos.xy / vPrevPos.w);
	vec2 v = abs(a - b);
	v *= uExposureTime;
	//v = uVel;

	// clamp to maximum length
	float originalLength = length(v);
	v *= min(uMaxVelocityMag / originalLength, 1.0);

	oVelocity = vec4(v, 0.0, 0.0);
}