#version 450 core

layout(location = 0) out vec4 oFragColor;

in vec3 vNormal;

layout(binding = 13) uniform sampler2D uLightProbe;

uniform bool uOct = false;

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

void main()
{
	oFragColor = vec4(textureLod(uLightProbe, octEncode(vNormal) * 0.5 + 0.5, 0).rgb, 1.0);
}