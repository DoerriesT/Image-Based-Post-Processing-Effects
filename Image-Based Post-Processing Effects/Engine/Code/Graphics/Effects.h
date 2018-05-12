#pragma once

struct FXAA
{
	bool enabled;
	float subPixelAA;
	float edgeThreshold;
	float edgeThresholdMin;
};

struct BokehDepthOfField
{
	bool enabled;
	float aperture;
	float apertureMax;
	float focalLength;
	float fStopsMin;
	float fStopsMax;
	float shutterAngleMax;
	unsigned int blades;
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
	bool enabled;
	unsigned int kernelSize;
	float radius;
	float bias;
};

struct ScreenSpaceReflections
{
	bool enabled;
};

enum class ShadowQuality
{
	OFF, NORMAL, HIGH
};

struct Effects
{
	SSAO ssao;
	Bloom bloom;
	Vignette vignette;
	FilmGrain filmGrain;
	ChromaticAberration chromaticAberration;
	FXAA fxaa;
	BokehDepthOfField depthOfField;
	LensFlares lensFlares;
	ShadowQuality shadowQuality;
	ScreenSpaceReflections screenSpaceReflections;
	float exposure;
};