#version 450 core

// Use a real macro here for maximum performance!
#ifndef SMAA_RT_METRICS // This is just for compilation-time syntax checking.
#define SMAA_RT_METRICS uResolution
#endif

// Set the HLSL version:
#ifndef SMAA_GLSL_4
#define SMAA_GLSL_4
#endif

// Set preset defines:
#define SMAA_PRESET_ULTRA

uniform vec4 uResolution; // float4(1.0 / 1280.0, 1.0 / 720.0, 1280.0, 720.0)

#define SMAA_INCLUDE_VS 0

// And include our header!
#include "SMAA.h"

layout(location = 0) out vec4 oFragColor;

in vec2 vTexCoord;
in vec4 vOffset[3];

layout(binding = 0) uniform sampler2D uColorTexture;

void main()
{
	vec2 result = SMAAColorEdgeDetectionPS(vTexCoord, vOffset, uColorTexture);
	oFragColor = vec4(result, 0.0, 1.0);
}