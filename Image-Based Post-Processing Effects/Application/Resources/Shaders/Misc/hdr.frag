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
uniform float uBloomDirtStrength = 0.5;
uniform float uExposure = 1.0;
uniform float uVelocityScale;
uniform float uHalfPixelWidth = 0.0003125;
uniform float uKeyValue = 0.18;
uniform float uLensDirtStrength;
uniform vec3 uAnamorphicFlareColor = vec3(1.0);

const float MAX_SAMPLES = 32.0;
const float SOFT_Z_EXTENT = 0.1;
const float Z_NEAR = 0.1;
const float Z_FAR = 300.0;
const int TILE_SIZE = 40;
const float GAMMA = 1.5;
const float PI = 3.14159265359;

const float A = 0.15; // shoulder strength
const float B = 0.50; // linear strength
const float C = 0.10; // linear angle
const float D = 0.20; // toe strength
const float E = 0.02; // toe numerator
const float F = 0.30; // toe denominator
const vec3 W = vec3(11.2); // linear white point value

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

float zCompare(float a, float b)
{
	float result = (a - b) / min(a, b);
	return clamp(1.0 - result, 0.0, 1.0);
}

float softDepthCompare(float a, float b)
{
	return clamp(1.0 - (a - b) / SOFT_Z_EXTENT, 0.0, 1.0);
}

float cone(float dist, float velocityMag)
{
	return mix(0.0, clamp(1.0 - dist / velocityMag, 0.0, 1.0),  sign(velocityMag));
}

float cylinder(float dist, float velocityMag)
{
	return 1.0 - smoothstep(0.95 * velocityMag, 1.05 * velocityMag, dist);
}

float linearDepth(float depth)
{
    float z_n = 2.0 * depth - 1.0;
    return 2.0 * Z_NEAR * Z_FAR / (Z_FAR + Z_NEAR - z_n * (Z_FAR - Z_NEAR));
}

const float HASHSCALE1 = 443.8975;

float hash12(vec2 p)
{
	vec3 p3  = fract(vec3(p.xyx) * HASHSCALE1);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

// Jitter function for tile lookup
vec2 jitterTile(vec2 uv)
{
	float val = hash12(uv + vec2(2.0, 0.0)) * PI * 2.0;
	return vec2(sin(val), cos(val)) * (1.0 / textureSize(uVelocityNeighborMaxTexture, 0).xy) * 0.25;
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
	vec2 texSize = textureSize(uScreenTexture, 0);
	
	vec2 velocity = texture(uVelocityTexture, vTexCoord).rg * texSize;

	float speed = length(velocity);
	float sampleCount = min(floor(speed), MAX_SAMPLES);
	
	if(sampleCount >= 1.0)
	{
		color = vec3(0.0);
		for (float i = 0; i < sampleCount; ++i) 
		{
			float t = mix(-1.0, 1.0, i / (sampleCount -1.0));
			ivec2 offset = ivec2(velocity * t);
			color += texelFetch(uScreenTexture, ivec2(gl_FragCoord.xy) + offset, 0).rgb;
		}
		
		color /= sampleCount;
	}
#elif MOTION_BLUR == 2
	vec2 texSize = textureSize(uScreenTexture, 0);
	
	vec2 vN = texture(uVelocityNeighborMaxTexture, vTexCoord).rg * texSize;

	if(dot(vN, vN) > 0.25)
	{
		vec2 vC = texelFetch(uVelocityTexture, ivec2(gl_FragCoord.xy), 0).rg * texSize;

		float depthC = -linearDepth(texelFetch(uDepthTexture, ivec2(gl_FragCoord.xy), 0).x);

		float vCLength = max(length(vC), 0.5);
		float vNLength = length(vN);

		float weight = 1.0 / vCLength;
		vec3 sum = color * weight;

		float j = 0.0;

		const int N = 25;

		for (int i = 0; i < N; ++i)
		{
			if (i == N / 2)
			{
				continue;
			}

			float t = mix(-1.0, 1.0, (i + j + 1.0) / (N + 1.0));
			ivec2 S = ivec2(gl_FragCoord.xy) + ivec2(vN * t + 0.5);

			float depthS = -linearDepth(texelFetch(uDepthTexture, S, 0).x);

			float f = softDepthCompare(depthC, depthS);
			float b = softDepthCompare(depthS, depthC);

			vec2 vS = texelFetch(uVelocityTexture, S, 0).rg * texSize;

			float vSLength = max(length(vS), 0.5);

			float dist = abs(t) * vNLength;

			float alpha = f * cone(dist, vSLength)
						+ b * cone(dist, vCLength)
						+ cylinder(dist, vSLength)
						* cylinder(dist, vCLength) * 2.0;

			weight += alpha;
			sum += alpha * texelFetch(uScreenTexture, S, 0).rgb;
		}
		
		color = sum / weight;
	}

#elif MOTION_BLUR == 3
	vec2 texSize = textureSize(uScreenTexture, 0);
	
	float j = 0.0;//(hash12(vTexCoord) - 0.5) * 2.0;
	vec2 vmax = texture(uVelocityNeighborMaxTexture, vTexCoord + jitterTile(vTexCoord)).rg * texSize;
	float vmaxLength = length(vmax);

	if(vmaxLength > 0.5)
	{
		vec2 wN = vmax / vmaxLength;
		vec2 vC = texelFetch(uVelocityTexture, ivec2(gl_FragCoord.xy), 0).rg * texSize;
		vec2 wP = vec2(-wN.y, wN.x);
		wP = (dot(wP, vC) < 0.0) ? -wP : wP;

		float vCLength = max(length(vC), 0.5);
		const float velocityThreshold = 1.5;
		vec2 wC = normalize(mix(wP, vC / vCLength, (vCLength - 0.5) / velocityThreshold));

		const float N = 25;
		
		float totalWeight = N / (TILE_SIZE * vCLength);
		vec3 result = color * totalWeight;
		
		float depthC = -linearDepth(texelFetch(uDepthTexture, ivec2(gl_FragCoord.xy), 0).x);

		vec2 dN[2] = { wN, vC / vCLength };

		for (int i = 0; i < N; ++i)
		{
			float t = mix(-1.0, 1.0, (i + j * 0.95 + 1.0) / (N + 1.0));
		
			vec2 d = bool(i & 1) ? vC : vmax;
			float T = abs(t) * vmaxLength;
			ivec2 S = ivec2(gl_FragCoord.xy) + ivec2(t * d);
		
			float depthS = -linearDepth(texelFetch(uDepthTexture, S, 0).x);
		
			float f = softDepthCompare(depthC, depthS);
			float b = softDepthCompare(depthS, depthC);
		
			vec2 vS = texelFetch(uVelocityTexture, S, 0).rg * texSize;

			float vSLength = max(length(vS), 0.5);

			int index = i & 1;
			float wA = abs(dot(wC, dN[index]));
			float wB = abs(dot(vS / vSLength, dN[index]));
			
			float weight = 0.0;

			weight += f * cone(T, vSLength) * wB;
			weight += b * cone(T, vCLength) * wA;
			// originally max(wA, wB), but using min reduces artifacts in extreme conditions
			weight += cylinder(T, min(vSLength, vCLength)) * min(wA, wB) * 2.0;
		
			totalWeight += weight;
			result += texelFetch(uScreenTexture, S, 0).rgb * weight;
		}
		
		color = result / totalWeight;
	}

#endif // MOTION_BLUR

	vec3 additions = vec3(0.0);

#if BLOOM_ENABLED
	vec3 lensMod = texture(uLensDirtTex, vTexCoord).rgb;
	vec3 bloom = texture(uBloomTexture, vTexCoord).rgb * uBloomStrength;
	additions += bloom;
#endif // BLOOM_ENABLED

#if FLARES_ENABLED
	vec2 centerVec = vTexCoord - vec2(0.5);
	float d = length(centerVec);
	float radial = acos(centerVec.x / d);
	float mask = texture(uLensStarTex, vec2(radial + uStarburstOffset * 1.0, 0.0)).r
				* texture(uLensStarTex, vec2(radial - uStarburstOffset * 0.5, 0.0)).r;
	
	mask = clamp(mask + (1.0 - smoothstep(0.0, 0.3, d)), 0.0, 1.0);

	vec3 lensFlare = texture(uLensFlareTex, vTexCoord).rgb * mask;
	additions += lensFlare;//vec3(1.0) - exp(-lensFlare * 0.15 * dot(lensFlare, vec3(0.299, 0.587, 0.114)));
#endif // FLARES_ENABLED

#if ANAMORPHIC_FLARES_ENABLED
		additions.rgb += texture(uAnamorphicTexture, vTexCoord).rgb * uAnamorphicFlareColor;
#endif // ANAMORPHIC_FLARES_ENABLED

#if DIRT_ENABLED
	vec3 dirt = texture(uLensDirtTex, vTexCoord).rgb;
	additions += additions * uLensDirtStrength * dirt;
#endif // DIRT_ENABLED

#if GOD_RAYS_ENABLED
	additions.rgb += texture(uGodRayTexture, vTexCoord).rgb;
#endif // GOD_RAYS_ENABLED

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