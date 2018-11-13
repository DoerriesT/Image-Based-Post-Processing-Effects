#version 450 core

#ifndef BLOOM_ENABLED
#define BLOOM_ENABLED 0
#endif // BLOOM_ENABLED

#ifndef FLARES_ENABLED
#define FLARES_ENABLED 0
#endif // FLARES_ENABLED

#ifndef ANAMORPHIC_FLARES_ENABLED
#define ANAMORPHIC_FLARES_ENABLED 0
#endif // ANAMORPHIC_FLARES_ENABLED

#ifndef DIRT_ENABLED
#define DIRT_ENABLED 0
#endif // DIRT_ENABLED

#ifndef GOD_RAYS_ENABLED
#define GOD_RAYS_ENABLED 0
#endif // GOD_RAYS_ENABLED

#ifndef AUTO_EXPOSURE_ENABLED
#define AUTO_EXPOSURE_ENABLED 0
#endif // AUTO_EXPOSURE_ENABLED

#ifndef MOTION_BLUR
#define MOTION_BLUR 1
#endif // MOTION_BLUR

out vec4 oFragColor;
in vec2 vTexCoord;

layout(binding = 0) uniform sampler2D uScreenTexture; // full resolution source texture
layout(binding = 1) uniform sampler2D uDepthTexture;
layout(binding = 2) uniform sampler2D uBloomTexture;
layout(binding = 3) uniform sampler2D uLensFlareTex; // input from the blur stage
layout(binding = 4) uniform sampler2D uLensDirtTex; // full resolution dirt texture
layout(binding = 5) uniform sampler2D uLensStarTex; // diffraction starburst texture
layout(binding = 6) uniform sampler2D uVelocityTexture;
layout(binding = 7) uniform sampler2D uVelocityNeighborMaxTexture;
layout(binding = 8) uniform sampler2D uLuminanceTexture;
layout(binding = 9) uniform sampler2D uGodRayTexture;
layout(binding = 10) uniform sampler2D uAnamorphicTexture;

uniform float uStarburstOffset; // transforms texcoords
uniform float uBloomStrength = 0.1;
uniform float uExposure = 1.0;
uniform float uKeyValue = 0.18;
uniform float uLensDirtStrength;
uniform vec3 uAnamorphicFlareColor = vec3(1.0);

const float Z_NEAR = 0.1;
const float Z_FAR = 300.0;
const float PI = 3.14159265359;

const float A = 0.15; // shoulder strength
const float B = 0.50; // linear strength
const float C = 0.10; // linear angle
const float D = 0.20; // toe strength
const float E = 0.02; // toe numerator
const float F = 0.30; // toe denominator
const vec3 W = vec3(11.2); // linear white point value

#define TILE_SIZE (40)
#define MAX_SAMPLES (25)

#include "motionBlur.h"

vec3 accurateLinearToSRGB(in vec3 linearCol)
{
	vec3 sRGBLo = linearCol * 12.92;
	vec3 sRGBHi = (pow(abs(linearCol), vec3(1.0/2.4)) * 1.055) - 0.055;
	vec3 sRGB = mix(sRGBLo, sRGBHi, vec3(greaterThan(linearCol, vec3(0.0031308))));
	return sRGB;
}

vec3 uncharted2Tonemap(vec3 x)
{
   return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

vec3 calculateExposedColor(vec3 color, float avgLuminance)
{
	// Use geometric mean
	avgLuminance = max(avgLuminance, 0.001f);
	float linearExposure = (uKeyValue / avgLuminance);
	float exposure = log2(max(linearExposure, 0.0001f));
    return exp2(exposure) * color;
}

float computeEV100(float aperture, float shutterTime, float ISO)
{
	// EV number is defined as:
	// 2^EV_s = N^2 / t and EV_s = EV_100 + log2(S/100)
	// This gives
	//   EV_s = log2(N^2 / t)
	//   EV_100 + log2(S/100) = log(N^2 / t)
	//   EV_100 = log2(N^2 / t) - log2(S/100)
	//   EV_100 = log2(N^2 / t * 100 / S)
	return log2((aperture * aperture) / shutterTime * 100 / ISO);
}

float computeEV100FromAvgLuminance(float avgLuminance)
{
	// We later use the middle gray at 12.7% in order to have
	// a middle gray at 12% with a sqrt(2) room for specular highlights
	// But here we deal with the spot meter measuring the middle gray
	// which is fixed at 12.5 for matching standard camera
	// constructor settings (i.e. calibration constant K = 12.5)
	// Reference: http://en.wikipedia.org/wiki/Film_speed
	return log2(max(avgLuminance, 1e-5) * 100.0 / 12.5);
}

float convertEV100ToExposure(float EV100)
{
	// Compute the maximum luminance possible with H_sbs sensitivity
	// maxLum = 78 / ( S * q ) * N^2 / t
	//        = 78 / ( S * q ) * 2^EV_100
	//        = 78 / ( 100 * 0.65) * 2^EV_100
	//        = 1.2 * 2^EV
	// Reference: http://en.wikipedia.org/wiki/Film_speed
	float maxLuminance = 1.2 * pow(2.0, EV100);
	return 1.0 / maxLuminance;
}

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

	vec3 additions = vec3(0.0);

#if BLOOM_ENABLED
	additions += texture(uBloomTexture, vTexCoord).rgb * uBloomStrength;
#endif // BLOOM_ENABLED

#if FLARES_ENABLED
	vec2 centerVec = vTexCoord - vec2(0.5);
	float d = length(centerVec);
	float radial = acos(centerVec.x / d);
	float mask = texture(uLensStarTex, vec2(radial + uStarburstOffset * 1.0, 0.0)).r
				* texture(uLensStarTex, vec2(radial - uStarburstOffset * 0.5, 0.0)).r;
	
	mask = clamp(mask + (1.0 - smoothstep(0.0, 0.3, d)), 0.0, 1.0);

	additions += texture(uLensFlareTex, vTexCoord).rgb * mask;
#endif // FLARES_ENABLED

#if ANAMORPHIC_FLARES_ENABLED
	additions.rgb += texture(uAnamorphicTexture, vTexCoord).rgb * uAnamorphicFlareColor;
#endif // ANAMORPHIC_FLARES_ENABLED

#if GOD_RAYS_ENABLED
	additions.rgb += texture(uGodRayTexture, vTexCoord).rgb;
#endif // GOD_RAYS_ENABLED

#if DIRT_ENABLED
	vec3 dirt = texture(uLensDirtTex, vTexCoord).rgb;
	additions += additions * uLensDirtStrength * dirt;
#endif // DIRT_ENABLED

	color.rgb += additions;
	
	// HDR tonemapping
	float exposureBias = 1.0;

#if AUTO_EXPOSURE_ENABLED
	float avgLuminance = texelFetch(uLuminanceTexture, ivec2(0, 0), 0).x;
	//float EV100 = computeEV100(16.0, 0.01, 100);
	float EV100 = computeEV100FromAvgLuminance(avgLuminance);
	EV100 -= 1.0;
	float exposure = convertEV100ToExposure(EV100);
	color *= exposure;//calculateExposedColor(color, avgLuminance);
#else
	color *= uExposure;
#endif // AUTO_EXPOSURE_ENABLED

	
	
	color = uncharted2Tonemap(exposureBias * color);
	vec3 whiteScale = 1.0/uncharted2Tonemap(W);
	color *= whiteScale;
    // gamma correct
    color = accurateLinearToSRGB(color);//pow(color, vec3(1.0/2.2));

	oFragColor = vec4(color, dot(color.rgb, vec3(0.299, 0.587, 0.114)));
}