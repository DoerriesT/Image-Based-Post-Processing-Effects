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

uniform vec4 uResolution; // vec4(1.0 / 1280.0, 1.0 / 720.0, 1280.0, 720.0)
uniform bool uTemporalSample;
uniform bool uTemporalAA;

#define SMAA_INCLUDE_VS 0

// And include our header!
#include "SMAA.h"

layout(location = 0) out vec4 oFragColor;

in vec2 vTexCoord;
in vec2 vPixCoord;
in vec4 vOffset[3];

layout(binding = 0) uniform sampler2D uEdgesTex;
layout(binding = 1) uniform sampler2D uAreaTex;
layout(binding = 2) uniform sampler2D uSearchTex;

void main()
{
	vec4 subsampleIndices = uTemporalAA ? uTemporalSample ? vec4(2, 2, 2, 0) : vec4(1, 1, 1, 0) : vec4(0.0);
	oFragColor = SMAABlendingWeightCalculationPS(vTexCoord, vPixCoord, vOffset, uEdgesTex, uAreaTex, uSearchTex, subsampleIndices);
}

