#pragma once

struct FXAA
{
	bool enabled;
	float subPixelAA;
	float edgeThreshold;
	float edgeThresholdMin;
};

struct LensFlares
{
	bool enabled;
	unsigned int flareCount;
	float flareSpacing;
	float haloWidth;
	float chromaticDistortion;
};

struct Bloom
{
	bool enabled;
	float strength;
	float lensDirtStrength;
};

struct ChromaticAberration
{
	bool enabled;
	float offsetMultiplier;
};

struct Vignette
{
	bool enabled;
};

struct FilmGrain
{
	bool enabled;
	float strength;
};

struct SSAO
{
	unsigned int kernelSize;
	float radius;
	float bias;
	float strength;
};

struct HBAO
{
	unsigned int directions;
	unsigned int steps;
	float strength;
	float radius;
	float maxRadiusPixels;
	float angleBias;
};

enum class AmbientOcclusion
{
	OFF, SSAO_ORIGINAL, SSAO, HBAO
};

struct ScreenSpaceReflections
{
	bool enabled;
};

enum class ShadowQuality
{
	OFF, NORMAL, HIGH
};

enum class MotionBlur
{
	OFF, SIMPLE, TILE_BASED_SINGLE, TILE_BASED_MULTI
};

enum class DepthOfField
{
	OFF, SIMPLE, SPRITE_BASED, TILE_BASED_SEPERATE, TILE_BASED_COMBINED
};

struct Effects
{
	AmbientOcclusion ambientOcclusion;
	SSAO ssao;
	HBAO hbao;
	Bloom bloom;
	Vignette vignette;
	FilmGrain filmGrain;
	ChromaticAberration chromaticAberration;
	FXAA fxaa;
	DepthOfField depthOfField;
	LensFlares lensFlares;
	ShadowQuality shadowQuality;
	ScreenSpaceReflections screenSpaceReflections;
	MotionBlur motionBlur;
	float exposure;
};