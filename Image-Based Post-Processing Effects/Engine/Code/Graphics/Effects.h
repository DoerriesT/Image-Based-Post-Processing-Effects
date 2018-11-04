#pragma once
#include <glm/vec3.hpp>

struct FXAA
{
	bool m_enabled;
	float m_subPixelAA;
	float m_edgeThreshold;
	float m_edgeThresholdMin;
};

struct SMAA
{
	bool m_enabled;
	bool m_temporalAntiAliasing;
};

struct LensFlares
{
	bool m_enabled;
	unsigned int m_flareCount;
	float m_flareSpacing;
	float m_haloWidth;
	float m_chromaticDistortion;
};

struct AnamorphicFlares
{
	bool m_enabled;
	glm::vec3 m_color;
};

struct Bloom
{
	bool m_enabled;
	float m_strength;
	
};

struct LensDirt
{
	bool m_enabled;
	float m_strength;
};

struct ChromaticAberration
{
	bool m_enabled;
	float m_offsetMultiplier;
};

struct Vignette
{
	bool m_enabled;
};

struct FilmGrain
{
	bool m_enabled;
	float m_strength;
};

struct SSAO
{
	unsigned int m_kernelSize;
	float m_radius;
	float m_bias;
	float m_strength;
};

struct HBAO
{
	unsigned int m_directions;
	unsigned int m_steps;
	float m_strength;
	float m_radius;
	float m_maxRadiusPixels;
	float m_angleBias;
};

struct GTAO
{
	unsigned int m_steps;
	float m_strength;
	float m_radius;
	float m_maxRadiusPixels;
};

enum class AmbientOcclusion
{
	OFF, SSAO_ORIGINAL, SSAO, HBAO, GTAO
};

enum class ShadowQuality
{
	OFF, NORMAL
};

enum class MotionBlur
{
	OFF, SIMPLE, TILE_BASED_SINGLE, TILE_BASED_MULTI
};

enum class DepthOfField
{
	OFF, SIMPLE, SPRITE_BASED, TILE_BASED
};

enum class DiffuseAmbientSource
{
	FLAT, IRRADIANCE_VOLUMES, LIGHT_PROPAGATION_VOLUMES
};

struct Effects
{
	AmbientOcclusion m_ambientOcclusion;
	SSAO m_ssao;
	HBAO m_hbao;
	GTAO m_gtao;
	Bloom m_bloom;
	LensDirt m_lensDirt;
	Vignette m_vignette;
	FilmGrain m_filmGrain;
	ChromaticAberration m_chromaticAberration;
	FXAA m_fxaa;
	SMAA m_smaa;
	DepthOfField m_depthOfField;
	LensFlares m_lensFlares;
	AnamorphicFlares m_anamorphicFlares;
	ShadowQuality m_shadowQuality;
	MotionBlur m_motionBlur;
	float m_exposure;
	DiffuseAmbientSource m_diffuseAmbientSource;
	bool m_godrays;
};

enum class GBufferDisplayMode
{
	SHADED, ALBEDO, NORMAL, MATERIAL, DEPTH, VELOCITY, AMBIENT_OCCLUSION
};