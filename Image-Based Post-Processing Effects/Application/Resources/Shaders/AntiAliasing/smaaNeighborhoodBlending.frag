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

#define SMAA_REPROJECTION 1
#define SMAA_INCLUDE_VS 0

// And include our header!
#include "SMAA.h"

layout(location = 0) out vec4 oFragColor;

in vec2 vTexCoord;
in vec4 vOffset;

layout(binding = 0) uniform sampler2D uColorTex;
layout(binding = 1) uniform sampler2D uBlendTex;
layout(binding = 2) uniform sampler2D uVelocityTex;

void main()
{
	oFragColor = SMAANeighborhoodBlendingPS(vTexCoord, vOffset, uColorTex, uBlendTex
	#if SMAA_REPROJECTION
	, uVelocityTex
	#endif
	);
}