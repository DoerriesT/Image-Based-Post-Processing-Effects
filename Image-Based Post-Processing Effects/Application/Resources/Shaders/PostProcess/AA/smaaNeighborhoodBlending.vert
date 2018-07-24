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

#define SMAA_INCLUDE_PS 0

// And include our header!
#include "SMAA.h"

out vec2 vTexCoord;
out vec4 vOffset;

void main()
{
	float x = -1.0 + float((gl_VertexID & 1) << 2);
	float y = -1.0 + float((gl_VertexID & 2) << 1);
	vTexCoord.x = (x+1.0)*0.5;
	vTexCoord.y = (y+1.0)*0.5;
	gl_Position = vec4(x, y, 0, 1);
	
	SMAANeighborhoodBlendingVS(vTexCoord, vOffset);
}