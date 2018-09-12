float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a2 = roughness*roughness;
    a2 *= a2;
    float NdotH2 = max(dot(N, H), 0.0);
    NdotH2 *= NdotH2;

    float nom   = a2;
    float denom = NdotH2 * (a2 - 1.0) + 1.0;

    denom = PI * denom * denom;

    return nom / max(denom, 0.0000001);
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
	float power = (-5.55473 * HdotV - 6.98316) * HdotV;
	return F0 + (1.0 - F0) * pow(2.0, power);
}

vec3 fresnelSchlickRoughness(float HdotV, vec3 F0, float roughness)
{
	float power = (-5.55473 * HdotV - 6.98316) * HdotV;
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(2.0, power);
}


// The code below is taken from "Moving Frostbite to PBR" Siggraph 2014 course notes

vec3 F_Schlick(in vec3 f0, in float f90, in float u)
{
	return f0 + (f90 - f0) * pow(1.0 - u, 5.0);
}

float V_SmithGGXCorrelated(float NdotL, float NdotV, float alphaG)
{
	// Original formulation of G_SmithGGX Correlated
	// lambda_v				= (-1 + sqrt(alphaG2 * (1 - NdotL2) / NdotL2 + 1)) * 0.5;
	// lambda_l				= (-1 + sqrt(alphaG2 * (1 - NdotV2) / NdotV2 + 1)) * 0.5;
	// G_SmithGGXCorrelated = 1 / (1 + lambda_v + lambda_l);
	// V_SmithGGXCorrelated = G_SmithGGXCorrelated / (4.0 * NdotL * NdotV);

	// This is the optimized version
	float alphaG2 = alphaG * alphaG;
	// Caution: the "NdotL *" and "NdotV *" are explicitely inversed, this is not a mistake
	float Lambda_GGXV = NdotL * sqrt((-NdotV * alphaG2 + NdotV) * NdotV + alphaG2);
	float Lambda_GGXL = NdotV * sqrt((-NdotL * alphaG2 + NdotL) * NdotL + alphaG2);

	return 0.5 / (Lambda_GGXV + Lambda_GGXL);
}

float D_GGX(float NdotH, float m)
{
	// Divide by PI is applied later
	float m2 = m * m;
	float f = (NdotH * m2 - NdotH) * NdotH + 1.0;
	return m2 / (f * f);
}

float Fr_DisneyDiffuse(float NdotV, float NdotL, float LdotH, float linearRoughness)
{
	float energyBias = mix(0.0, 0.5, linearRoughness);
	float energyFactor = mix(1.0, 1.0 / 1.51, linearRoughness);
	float fd90 = energyBias + 2.0 * LdotH * LdotH * linearRoughness;
	vec3 f0 = vec3(1.0);
	float lightScatter = F_Schlick(f0, fd90, NdotL).r;
	float viewScatter = F_Schlick(f0, fd90, NdotV).r;

	return lightScatter * viewScatter * energyFactor;
}