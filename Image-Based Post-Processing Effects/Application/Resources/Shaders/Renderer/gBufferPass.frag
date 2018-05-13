#version 330 core

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
    float ao;
	vec3 emissive;
    int mapBitField;
	sampler2D albedoMap;
	sampler2D normalMap;
	sampler2D metallicMap;
	sampler2D roughnessMap;
	sampler2D aoMap;
	sampler2D emissiveMap;
};

uniform Material uMaterial;

vec2 encode (vec3 n)
{
    float f = sqrt(8.0 * n.z + 8.0);
    return n.xy / f + 0.5;
}

void main()
{
    if((uMaterial.mapBitField & ALBEDO) != 0)
    {
		vec3 sample = texture(uMaterial.albedoMap, vTexCoord).rgb;
		oAlbedo = vec4(sample, 1.0);
    }
	else
	{
		oAlbedo = uMaterial.albedo;
	}

    vec3 N = normalize(vNormal);
    if((uMaterial.mapBitField & NORMAL) != 0)
    {
        vec3 tangentNormal = texture(uMaterial.normalMap, vTexCoord).xyz * 2.0 - 1.0;
        N = normalize(mat3(normalize(vTangent), -normalize(vBitangent), N)*tangentNormal);
    }
	oNormal = vec4(encode(N), 0.0, 0.0);
	

    if((uMaterial.mapBitField & METALLIC) != 0)
    {
        oMetallicRoughnessAo.r  = texture(uMaterial.metallicMap, vTexCoord).r;
    }
    else
    {
        oMetallicRoughnessAo.r = uMaterial.metallic;
    }

    if((uMaterial.mapBitField & ROUGHNESS) != 0)
    {
        oMetallicRoughnessAo.g = texture(uMaterial.roughnessMap, vTexCoord).r;
    }
    else
    {
        oMetallicRoughnessAo.g = uMaterial.roughness;
    }

    if((uMaterial.mapBitField & AO) != 0)
    {
        oMetallicRoughnessAo.b = texture(uMaterial.aoMap, vTexCoord).r;
    }
    else
    {
        oMetallicRoughnessAo.b = uMaterial.ao;
    }

	if((uMaterial.mapBitField & EMISSIVE) != 0)
    {
        oEmissive = vec4(uMaterial.emissive * texture(uMaterial.emissiveMap, vTexCoord).rgb, 1.0);
    }
    else
    {
        oEmissive = vec4(uMaterial.emissive, 0.0);
    }

	oMetallicRoughnessAo.a = 1.0;

	vec2 a = (vCurrentPos.xy / vCurrentPos.w);
    vec2 b = (vPrevPos.xy / vPrevPos.w);
	vec2 v = abs(a - b);
	//v = pow(v, vec2(3.0));
	oVelocity = vec4(v, 0.0, 0.0); // vec4(a - b, 0.0, 0.0); //vec4(pow((a - b) * 0.5 + 0.5, vec2(3.0)), 0.0, 0.0);
}