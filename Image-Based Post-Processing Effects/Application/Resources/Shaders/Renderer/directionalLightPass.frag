#version 450 core

const float PI = 3.14159265359;

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
layout(binding = 4) uniform sampler2DArrayShadow uShadowMap;

uniform DirectionalLight uDirectionalLight;
uniform mat4 uInverseView;
uniform mat4 uInverseProjection;
uniform bool uShadowsEnabled;

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
	float power = (-5.55473 * HdotV - 6.98316) * HdotV;
	return F0 + (1.0 - F0) * pow(2.0, power);
}

void main()
{
	vec2 texCoord = gl_FragCoord.xy /  textureSize(uAlbedoMap, 0);

	vec3 albedo = texture(uAlbedoMap, texCoord).rgb;
	
	vec4 metallicRoughnessAoShaded = texture(uMetallicRoughnessAoMap, texCoord).rgba;
    
		
    if (metallicRoughnessAoShaded.a > 0.0)
    {
		vec3 N = decode(texture(uNormalMap, texCoord).xy);
		
		float depth = texture(uDepthMap, texCoord).r;
		vec4 clipSpacePosition = vec4(vTexCoord * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
		vec4 viewSpacePosition = uInverseProjection * clipSpacePosition;
		viewSpacePosition /= viewSpacePosition.w;

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

		vec3 V = -normalize(viewSpacePosition.xyz);
		vec3 L = normalize(uDirectionalLight.direction);
		vec3 H = normalize(V + L);

		float NdotV = max(dot(N, V), 0.0);
		float NdotL = max(dot(N, L), 0.0);
		
		// Cook-Torrance BRDF
		float NDF = DistributionGGX(N, H, metallicRoughnessAoShaded.g);
		float G = GeometrySmith(NdotV, NdotL, metallicRoughnessAoShaded.g);
		vec3 F0 = mix(vec3(0.04), albedo, metallicRoughnessAoShaded.r);
		vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

		vec3 nominator = NDF * G * F;
		float denominator = max(4 * NdotV * NdotL, 0.0000001);

		vec3 specular = nominator / denominator;

		// because of energy conversion kD and kS must add up to 1.0
		vec3 kD = vec3(1.0) - F;
		// multiply kD by the inverse metalness so if a material is metallic, it has no diffuse lighting (and otherwise a blend)
		kD *= 1.0 - metallicRoughnessAoShaded.r;

		oFragColor = metallicRoughnessAoShaded.a * vec4((kD * albedo.rgb / PI + specular) * uDirectionalLight.color * NdotL * (1.0 - shadow), 1.0);
    }
	else
	{
		oFragColor = vec4(albedo, 1.0);
	}
}