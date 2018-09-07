#version 450 core

layout(location = 0) out vec4 oFragColor;

layout(early_fragment_tests) in;

in vec3 vNormal;

layout(binding = 13) uniform sampler2D uLightProbe;

uniform bool uSH;

uniform int uIndex;

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

const float Pi = 3.141592654f;
const float CosineA0 = Pi;
const float CosineA1 = (2.0f * Pi) / 3.0f;
const float CosineA2 = Pi * 0.25f;

struct SH9
{
    float c[9];
};

struct SH9Color
{
    vec3 c[9];
};

SH9 SHCosineLobe(in vec3 dir)
{
    SH9 sh;

    // Band 0
    sh.c[0] = 0.282095f * CosineA0;

    // Band 1
    sh.c[1] = 0.488603f * dir.y * CosineA1;
    sh.c[2] = 0.488603f * dir.z * CosineA1;
    sh.c[3] = 0.488603f * dir.x * CosineA1;

    // Band 2
    sh.c[4] = 1.092548f * dir.x * dir.y * CosineA2;
    sh.c[5] = 1.092548f * dir.y * dir.z * CosineA2;
    sh.c[6] = 0.315392f * (3.0f * dir.z * dir.z - 1.0f) * CosineA2;
    sh.c[7] = 1.092548f * dir.x * dir.z * CosineA2;
    sh.c[8] = 0.546274f * (dir.x * dir.x - dir.y * dir.y) * CosineA2;

    return sh;
}

vec3 ComputeSHIrradiance(in vec3 normal, in SH9Color radiance)
{
    // Compute the cosine lobe in SH, oriented about the normal direction
    SH9 shCosine = SHCosineLobe(normal);

    // Compute the SH dot product to get irradiance
    vec3 irradiance = vec3(0.0);
    for(uint i = 0; i < 9; ++i)
        irradiance += radiance.c[i] * shCosine.c[i];

    return irradiance;
}

void main()
{
	//if (uSH)
	//{
		SH9Color shColor;
		for (int i = 0; i < 9; ++i)
		{
			shColor.c[i] = texelFetch(uLightProbe, ivec2(i, uIndex), 0).rgb;
		}
		oFragColor = vec4(ComputeSHIrradiance(normalize(vNormal), shColor), 1.0);
	//}
	//else
	//{
	//	oFragColor = vec4(textureLod(uLightProbe, octEncode(normalize(vNormal)) * 0.5 + 0.5, 0).rgb, 1.0);
	//}
}