#version 450 core

#ifndef MOTION_BLUR
#define MOTION_BLUR 1
#endif // MOTION_BLUR

out vec4 oFragColor;
in vec2 vTexCoord;

layout(binding = 0) uniform sampler2D uScreenTexture; // full resolution source texture
layout(binding = 1) uniform sampler2D uDepthTexture;
layout(binding = 6) uniform sampler2D uVelocityTexture;
layout(binding = 7) uniform sampler2D uVelocityNeighborMaxTexture;


const float Z_NEAR = 0.1;
const float Z_FAR = 300.0;
const float PI = 3.14159265359;

#define TILE_SIZE (40)
#define MAX_SAMPLES (25)

#include "motionBlur.h"

void main()
{
	vec3 color = texture(uScreenTexture, vTexCoord).rgb;

#if MOTION_BLUR == 1
	color = simpleMotionBlur(color, vTexCoord, ivec2(gl_FragCoord.xy), uScreenTexture, uVelocityTexture);
#elif MOTION_BLUR == 2
	color = singleDirectionMotionBlur(color, vTexCoord, ivec2(gl_FragCoord.xy), uScreenTexture, uVelocityTexture, uDepthTexture, uVelocityNeighborMaxTexture, Z_NEAR, Z_FAR);
#elif MOTION_BLUR == 3
	color = multiDirectionMotionBlur(color, vTexCoord, ivec2(gl_FragCoord.xy), uScreenTexture, uVelocityTexture, uDepthTexture, uVelocityNeighborMaxTexture, Z_NEAR, Z_FAR);
#endif // MOTION_BLUR

	oFragColor = vec4(color, 1);
}