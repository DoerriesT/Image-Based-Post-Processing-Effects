#pragma once

struct FXAA
{
	bool enabled;
	float subPixelAA;
	float edgeThreshold;
	float edgeThresholdMin;
};

struct SMAA
{
	bool enabled;
	bool temporalAntiAliasing;
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
	
};

struct LensDirt
{
	bool enabled;
	float strength;
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

struct GTAO
{
	unsigned int steps;
	float strength;
	float radius;
	float maxRadiusPixels;
};

enum class AmbientOcclusion
{
	OFF, SSAO_ORIGINAL, SSAO, HBAO, GTAO
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

enum class DiffuseAmbientSource
{
	FLAT, IRRADIANCE_VOLUMES, LIGHT_PROPAGATION_VOLUMES
};

struct Effects
{
	AmbientOcclusion ambientOcclusion;
	SSAO ssao;
	HBAO hbao;
	GTAO gtao;
	Bloom bloom;
	LensDirt lensDirt;
	Vignette vignette;
	FilmGrain filmGrain;
	ChromaticAberration chromaticAberration;
	FXAA fxaa;
	SMAA smaa;
	DepthOfField depthOfField;
	LensFlares lensFlares;
	ShadowQuality shadowQuality;
	ScreenSpaceReflections screenSpaceReflections;
	MotionBlur motionBlur;
	float exposure;
	DiffuseAmbientSource diffuseAmbientSource;
};

enum class GBufferDisplayMode
{
	SHADED, ALBEDO, NORMAL, MATERIAL, DEPTH, VELOCITY, AMBIENT_OCCLUSION
};