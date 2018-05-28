#version 330 core

const float PI = 3.14159265359;
const float MAX_REFLECTION_LOD = 4.0;

layout(location = 0) out vec4 oFragColor;

in vec2 vTexCoord;

struct DirectionalLight
{
    vec3 color;
    vec3 direction;
	bool renderShadows;
	sampler2D shadowMap;
	mat4 viewProjectionMatrix;
};

uniform sampler2D uAlbedoMap;
uniform sampler2D uNormalMap;
uniform sampler2D uMetallicRoughnessAoMap;
uniform sampler2D uDepthMap;
uniform sampler2D uSsaoMap;
uniform sampler2D uPrevFrame;
uniform mat4 uInverseView;
uniform mat4 uProjection;
uniform mat4 uInverseProjection;
uniform mat4 uPrevViewProjection;
uniform DirectionalLight uDirectionalLight;
uniform bool uShadowsEnabled;
uniform bool uRenderDirectionalLight;
uniform bool uSsao;
uniform bool uUseSsr;

//IBL
uniform samplerCube uIrradianceMap;
uniform samplerCube uPrefilterMap;
uniform sampler2D uBrdfLUT;

const float stepLength = 0.04;
const float minRayStep = 0.1;
const float maxSteps = 30;
const int numBinarySearchSteps = 10;
const float reflectionSpecularFalloffExponent = 3.0;

#define Scale vec3(.8, .8, .8)
#define K 19.19

vec3 hash(vec3 a)
{
    a = fract(a * Scale);
    a += dot(a, a.yxz + K);
    return fract((a.xxy + a.yxx)*a.zyx);
}

vec3 binarySearch(inout vec3 dir, // ray direction
	inout vec3 hitCoord, // ray startpoint / resolut point 
	inout float dDepth)
{
    float depth;

    vec4 projectedCoord;
 
    for(int i = 0; i < numBinarySearchSteps; ++i)
    {
		// transform from viewspace to postprojective space
        projectedCoord = uProjection * vec4(hitCoord, 1.0);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
 
        // get depth at new position
		float depth = texture(uDepthMap, projectedCoord.xy).r;
		vec4 clipSpacePosition = vec4(projectedCoord.xy * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
		vec4 viewSpacePosition = uInverseProjection * clipSpacePosition;
		viewSpacePosition /= viewSpacePosition.w;
        depth = viewSpacePosition.z;

 
        dDepth = hitCoord.z - depth;

        dir *= 0.5;
        if(dDepth > 0.0)
		{
			hitCoord += dir;
		}
        else
		{
			hitCoord -= dir;   
		} 
    }

	/*
    projectedCoord = uProjection * vec4(hitCoord, 1.0);
    projectedCoord.xy /= projectedCoord.w;
    projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

	return vec3(projectedCoord.xy, depth);
	*/
	
	vec4 reprojectedCoord = uPrevViewProjection * uInverseView * vec4(hitCoord, 1.0);
	reprojectedCoord.xy /= reprojectedCoord.w;
	reprojectedCoord.xy = reprojectedCoord.xy * 0.5 + 0.5;
 
    return vec3(reprojectedCoord.xy, depth);
	
}

vec3 ssrRaymarch(
	vec3 dir, // ray direction
	inout vec3 hitCoord, // ray startpoint / resolut point 
	out float dDepth)
{
	dir *= stepLength;
 
    float depth;
    int steps;
    vec4 projectedCoord;

 
    for(int i = 0; i < maxSteps; ++i)
    {
		// marching step
        hitCoord += dir;
 
		// transform from viewspace to postprojective space
        projectedCoord = uProjection * vec4(hitCoord, 1.0);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
 
		// get depth at new position
		float depth = texture(uDepthMap, projectedCoord.xy).r;
		vec4 clipSpacePosition = vec4(projectedCoord.xy * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
		vec4 viewSpacePosition = uInverseProjection * clipSpacePosition;
		viewSpacePosition /= viewSpacePosition.w;
        depth = viewSpacePosition.z;

        if(depth > 1000.0)
        {
			continue;
		}
 
        dDepth = hitCoord.z - depth;

        if((dir.z - dDepth) < 2.2)
        {
            if(dDepth <= 0.0)
            {   
                return vec3(binarySearch(dir, hitCoord, dDepth));
            }
        }
        
        ++steps;
    }
 
    
    return vec3(projectedCoord.xy, depth);
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
	vec2 texCoord = gl_FragCoord.xy / textureSize(uAlbedoMap, 0);

	vec3 albedo = texture(uAlbedoMap, texCoord).rgb;
	
	vec4 metallicRoughnessAoShaded = texture(uMetallicRoughnessAoMap, texCoord).rgba;

	if(uSsao)
	{
		metallicRoughnessAoShaded.b = min(metallicRoughnessAoShaded.b, texture(uSsaoMap, texCoord).r);
	}
    
		
    if (metallicRoughnessAoShaded.a > 0.0)
    {
		vec3 N = decode(texture(uNormalMap, texCoord).xy);
		
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
				vec4 worldPos4 = uInverseView * viewSpacePosition;
				worldPos4 /= worldPos4.w;
				vec4 projCoords4 = uDirectionalLight.viewProjectionMatrix * worldPos4;
				vec3 projCoords = (projCoords4 / projCoords4.w).xyz;
				projCoords = projCoords * 0.5 + 0.5; 
				vec2 moments = texture(uDirectionalLight.shadowMap, projCoords.xy).xy;
				float currentDepth = projCoords.z;

				float p = (currentDepth <= moments.x) ? 1.0 : 0.0;
				float variance = moments.y - (moments.x * moments.x);
				variance = max(variance, 0.00002);
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

			vec3 irradiance = texture(uIrradianceMap, worldNormal).rgb;
			vec3 diffuse = irradiance * albedo;

			vec3 prefilteredColor = vec3(0.0);

			// Screenspace Reflections
			if(uUseSsr)
			{
				vec4 worldPos4 = uInverseView * viewSpacePosition;
				worldPos4 /= worldPos4.w;
				// start raymarching at current view space position
				vec3 ssrHitPos = viewSpacePosition.xyz;
				float dDepth = 0.0;
				// add jittering to ray to simulate roughness
				vec3 jittering = mix(vec3(0.0), hash(worldPos4.xyz), metallicRoughnessAoShaded.g);
				vec3 R = reflect(-V, N);
				vec3 coords = ssrRaymarch(jittering + R * max(minRayStep, -viewSpacePosition.z), ssrHitPos, dDepth);

				vec2 dCoords = smoothstep(0.2, 0.6, abs(vec2(0.5) - coords.xy));

				float screenEdgeFactor = clamp(1.0 - (dCoords.x + dCoords.y), 0.0, 1.0);

				float reflectionMultiplier = pow(metallicRoughnessAoShaded.r, reflectionSpecularFalloffExponent) * screenEdgeFactor * -R.z;

				// get reflected color from previous frame
				vec3 ssr = texture(uPrevFrame, coords.xy).rgb * clamp(reflectionMultiplier, 0.0, 0.9);

				// sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
				prefilteredColor = mix(textureLod(uPrefilterMap, (uInverseView * vec4(reflect(-V, N), 0.0)).xyz, metallicRoughnessAoShaded.g * MAX_REFLECTION_LOD).rgb, ssr, screenEdgeFactor);
			}
			else
			{
				// sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
				prefilteredColor = textureLod(uPrefilterMap, (uInverseView * vec4(reflect(-V, N), 0.0)).xyz, metallicRoughnessAoShaded.g * MAX_REFLECTION_LOD).rgb;
			}


			// TODO: find out why brdfLUT is weirdly sampled
			vec2 brdf  = texture(uBrdfLUT, vec2(NdotV, max(metallicRoughnessAoShaded.g, 0.004))).rg;
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