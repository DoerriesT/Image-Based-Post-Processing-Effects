#version 330 core

const float PI = 3.14159265359;
const int ALBEDO = 1;
const int NORMAL = 2;
const int METALLIC = 4;
const int ROUGHNESS = 8;
const int AO = 16;
const int EMISSIVE = 32;

layout(location = 0) out vec4 oFragColor;

in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
in vec4 vWorldPos;

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

struct DirectionalLight
{
    vec3 color;
    vec3 direction;
	bool renderShadows;
	sampler2D shadowMap;
	mat4 viewProjectionMatrix;
};

uniform Material uMaterial;
uniform DirectionalLight uDirectionalLight;
uniform bool uRenderDirectionalLight;
uniform vec3 uCamPos;
uniform bool uShadowsEnabled;

//IBL
uniform samplerCube uIrradianceMap;
uniform samplerCube uPrefilterMap;
uniform sampler2D uBrdfLUT;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a2 = roughness*roughness;
    a2 *= a2;
    float NdotH2 = max(dot(N, H), 0.0);
    NdotH2 *= NdotH2;

    float nom   = a2;
    float denom = max(NdotH2 * (a2 - 1.0) + 1.0, 0.0000001);

    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySmith(float NdotV, float NdotL, float roughness)
{
	float r = (roughness + 1.0);
    float k = (r*r) / 8.0;
    float ggx2 =  NdotV / max(NdotV * (1.0 - k) + k, 0.0000001);
    float ggx1 = NdotL / max(NdotL * (1.0 - k) + k, 0.0000001);

    return ggx1 * ggx2;
}


vec3 fresnelSchlick(float HdotV, vec3 F0)
{
	float fresnel = 1.0 - HdotV;
	fresnel *= fresnel;
	fresnel *= fresnel;
	return F0 + (1.0 - F0) * fresnel;
}

vec3 fresnelSchlickRoughness(float HdotV, vec3 F0, float roughness)
{
	float fresnel = 1.0 - HdotV;
	fresnel *= fresnel;
	fresnel *= fresnel;
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * fresnel;
}  

void main()
{
    vec4 albedo;
    if((uMaterial.mapBitField & ALBEDO) == ALBEDO)
    {
		albedo = texture(uMaterial.albedoMap, vTexCoord).rgba; 
        albedo.rgb = pow(albedo.rgb, vec3(2.2));
    }
    else
    {
        albedo = uMaterial.albedo;
    }

    vec3 N = normalize(vNormal);
    if((uMaterial.mapBitField & NORMAL) == NORMAL)
    {
        vec3 tangentNormal = texture(uMaterial.normalMap, vTexCoord).xyz * 2.0 - 1.0;
        N = normalize(mat3(normalize(vTangent), -normalize(vBitangent), N)*tangentNormal);
    }

    float metallic;
    if((uMaterial.mapBitField & METALLIC) == METALLIC)
    {
        metallic  = texture(uMaterial.metallicMap, vTexCoord).r;
    }
    else
    {
        metallic = uMaterial.metallic;
    }

    float roughness;
    if((uMaterial.mapBitField & ROUGHNESS) == ROUGHNESS)
    {
        roughness = texture(uMaterial.roughnessMap, vTexCoord).r;
    }
    else
    {
        roughness = uMaterial.roughness;
    }

    float ao;
    if((uMaterial.mapBitField & AO) == AO)
    {
        ao = texture(uMaterial.aoMap, vTexCoord).r;
    }
    else
    {
        ao = uMaterial.ao;
    }

	vec3 emissive;
	if((uMaterial.mapBitField & EMISSIVE) != 0)
    {
        emissive = texture(uMaterial.emissiveMap, vTexCoord).rgb;
    }
    else
    {
        emissive = uMaterial.emissive;
    }
	
	vec3 V = normalize(uCamPos - vWorldPos.xyz/vWorldPos.w);
	float NdotV = max(dot(N, V), 0.0);
	vec3 F0 = mix(vec3(0.04), albedo.rgb, metallic);
	
	vec3 directionalLightContribution = vec3(0.0);

	if (uRenderDirectionalLight)
	{
		// shadow
		float shadow = 0.0;
		if(uDirectionalLight.renderShadows && uShadowsEnabled)
		{	
			vec4 projCoords4 = uDirectionalLight.viewProjectionMatrix * vWorldPos;
			vec3 projCoords = (projCoords4 / projCoords4.w).xyz;
			projCoords = projCoords * 0.5 + 0.5; 
			vec2 moments = texture(uDirectionalLight.shadowMap, projCoords.xy).xy;
			float currentDepth = projCoords.z;

			float p = (currentDepth <= moments.x) ? 1.0 : 0.0;
			float variance = moments.y - (moments.x * moments.x);
			variance = max(variance, 0.00001);
			float d = currentDepth - moments.x;
			float p_max = variance / (variance + d * d);
			shadow = 1.0 - max(p, p_max);
			if(projCoords.z > 1.0)
			{
				shadow = 0.0;
			}
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

	vec3 irradiance = texture(uIrradianceMap, N).rgb;
	vec3 diffuse = irradiance * albedo.rgb;

	// sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
	const float MAX_REFLECTION_LOD = 4.0;
	vec3 prefilteredColor = textureLod(uPrefilterMap, reflect(-V, N), roughness * MAX_REFLECTION_LOD).rgb;
	// TODO: find out why brdfLUT is weirdly sampled
	vec2 brdf  = texture(uBrdfLUT, vec2(NdotV, max(roughness, 0.004))).rg;
	vec3 specular = prefilteredColor * (kS * brdf.x + brdf.y);
				
	vec3 ambientLightContribution = vec3((kD * diffuse + specular) * ao);

	oFragColor = vec4(directionalLightContribution + ambientLightContribution + emissive, albedo.a);
}