#version 330 core

layout(location = 0) out vec4 oColor;

in vec2 vTexCoord;

const float PI = 3.14159265359;

// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
// efficient VanDerCorpus calculation.
float RadicalInverse_VdC(uint bits) 
{
     bits = (bits << 16u) | (bits >> 16u);
     bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
     bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
     bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
     bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
     return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
	float a = roughness*roughness;
	
	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	
	// from spherical coordinates to cartesian coordinates - halfway vector
	vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;
	
	// from tangent-space H vector to world-space sample vector
	vec3 up          = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent   = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);
	
	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}

float GeometrySmith(float NdotV, float NdotL, float roughness)
{
	// note that we use a different k for IBL
    float k = (roughness * roughness) * 0.5;
    float ggx2 =  NdotV / max(NdotV * (1.0 - k) + k, 0.0000001);
    float ggx1 = NdotL / max(NdotL * (1.0 - k) + k, 0.0000001);

    return ggx1 * ggx2;
}

void main()
{
	float roughness = vTexCoord.x;
	float NdotV = vTexCoord.y;
	
	vec3 V;
	V.x = sqrt(1.0 - NdotV * NdotV); // sin
	V.y = 0.0;
	V.z = NdotV;
	
	float A = 0.0;
	float B = 0.0;
	
	const uint NUM_SAMPLES = 1024u;
	const vec3 N = vec3(0.0, 0.0, 1.0);
	
	for (uint i = 0; i < NUM_SAMPLES; ++i)
	{
		vec2 Xi = Hammersley(i, NUM_SAMPLES);
		vec3 H = ImportanceSampleGGX(Xi, N, roughness);
		vec3 L = normalize(2.0 * dot(V, H) * H - V);
		
		float NdotL = max(L.z, 0.0);

        if(NdotL > 0.0)
        {
			float NdotH = max(H.z, 0.0);
			float VdotH = max(dot(V, H), 0.0);
		
            float G = GeometrySmith(NdotV, NdotL, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0 - VdotH, 5.0);

            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
	}
	
	A /= float(NUM_SAMPLES);
    B /= float(NUM_SAMPLES);
	
	oColor = vec4(A, B, 0.0, 0.0);
	oColor = vec4(1.0);
}