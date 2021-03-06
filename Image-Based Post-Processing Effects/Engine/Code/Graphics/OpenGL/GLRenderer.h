#pragma once
#include <memory>
#include <glad\glad.h>
#include "GLTimerQuery.h"

struct GLRenderResources;
class Scene;
struct Effects;
struct Level;
class ShaderProgram;
struct RenderData;
struct AxisAlignedBoundingBox;
struct OceanParams;
class ShadowRenderPass;
class GBufferRenderPass;
class GBufferCustomRenderPass;
class SSAOOriginalRenderPass;
class SSAORenderPass;
class HBAORenderPass;
class GTAORenderPass;
class GTAOSpatialDenoiseRenderPass;
class GTAOTemporalDenoiseRenderPass;
class SSAOBlurRenderPass;
class SSAOBilateralBlurRenderPass;
class SkyboxRenderPass;
class AmbientLightRenderPass;
class DirectionalLightRenderPass;
class StencilRenderPass;
class DeferredEnvironmentProbeRenderPass;
class PointLightRenderPass;
class SpotLightRenderPass;
class ForwardRenderPass;
class ForwardCustomRenderPass;
class OutlineRenderPass;
class LightProbeRenderPass;
class AnamorphicPrefilterComputePass;
class AnamorphicDownsampleComputePass;
class AnamorphicUpsampleComputePass;
class FXAARenderPass;
class SMAAEdgeDetectionRenderPass;
class SMAABlendWeightRenderPass;
class SMAABlendRenderPass;
class SMAATemporalResolveRenderPass;
class GodRayMaskComputePass;
class GodRayGenComputePass;
class LuminanceGenComputePass;
class LuminanceAdaptionComputePass;
class VelocityCorrectionComputePass;
class SimpleDofCocBlurComputePass;
class SimpleDofBlurComputePass;
class SimpleDofFillComputePass;
class SimpleDofCompositeComputePass;
class SpriteDofRenderPass;
class SpriteDofCompositeComputePass;
class TileBasedDofDownsampleComputePass;
class TileBasedDofBlurComputePass;
class TileBasedDofFillComputePass;
class TileBasedDofCompositeComputePass;
class LuminanceHistogramComputePass;
class LuminanceHistogramReduceComputePass;
class LuminanceHistogramAdaptionComputePass;
class CocComputePass;
class CocTileMaxRenderPass;
class CocNeighborTileMaxRenderPass;
class VelocityTileMaxRenderPass;
class VelocityNeighborTileMaxRenderPass;
class LensFlareGenRenderPass;
class LensFlareBlurRenderPass;
class BloomDownsampleComputePass;
class BloomUpsampleComputePass;
class SimplePostEffectsRenderPass;
class ToneMapRenderPass;
class TileBasedDofTileMaxComputePass;
class AntiAliasingTonemapComputePass;
class AntiAliasingReverseTonemapComputePass;
class BoundingBoxRenderPass;
class MotionBlurRenderPass;

class GLRenderer
{
public:
	explicit GLRenderer();
	~GLRenderer();
	GLRenderer(const GLRenderer &) = delete;
	GLRenderer(const GLRenderer &&) = delete;
	GLRenderer &operator= (const GLRenderer &) = delete;
	GLRenderer &operator= (const GLRenderer &&) = delete;
	void init(unsigned int width, unsigned int height);
	void render(const RenderData &renderData, const Scene &scene, const std::shared_ptr<Level> &level, const Effects &effects, bool bake = false, bool debugDraw = false);
	void resize(unsigned int width, unsigned int height);
	GLuint getColorTexture() const;
	GLuint getAlbedoTexture() const;
	GLuint getNormalTexture() const;
	GLuint getMaterialTexture() const;
	GLuint getDepthStencilTexture() const;
	GLuint getVelocityTexture() const;
	GLuint getAmbientOcclusionTexture() const;
	GLuint getBrdfLUT() const;
	GLuint getFinishedTexture() const;

private:
	std::unique_ptr<GLRenderResources> m_renderResources;

	// scene rendering
	std::unique_ptr<ShadowRenderPass> m_shadowRenderPass;
	std::unique_ptr<GBufferRenderPass> m_gBufferRenderPass;
	std::unique_ptr<GBufferCustomRenderPass> m_gBufferCustomRenderPass;
	std::unique_ptr<SSAOOriginalRenderPass> m_ssaoOriginalRenderPass;
	std::unique_ptr<SSAORenderPass> m_ssaoRenderPass;
	std::unique_ptr<HBAORenderPass> m_hbaoRenderPass;
	std::unique_ptr<GTAORenderPass> m_gtaoRenderPass;
	std::unique_ptr<GTAOSpatialDenoiseRenderPass> m_gtaoSpatialDenoiseRenderPass;
	std::unique_ptr<GTAOTemporalDenoiseRenderPass> m_gtaoTemporalDenoiseRenderPass;
	std::unique_ptr<SSAOBlurRenderPass> m_ssaoBlurRenderPass;
	std::unique_ptr<SSAOBilateralBlurRenderPass> m_ssaoBilateralBlurRenderPass;
	std::unique_ptr<SkyboxRenderPass> m_skyboxRenderPass;
	std::unique_ptr<AmbientLightRenderPass> m_ambientLightRenderPass;
	std::unique_ptr<DirectionalLightRenderPass> m_directionalLightRenderPass;
	std::unique_ptr<StencilRenderPass> m_stencilRenderPass;
	std::unique_ptr<DeferredEnvironmentProbeRenderPass> m_deferredEnvironmentProbeRenderPass;
	std::unique_ptr<PointLightRenderPass> m_pointLightRenderPass;
	std::unique_ptr<SpotLightRenderPass> m_spotLightRenderPass;
	std::unique_ptr<ForwardRenderPass> m_forwardRenderPass;
	std::unique_ptr<ForwardCustomRenderPass> m_forwardCustomRenderPass;
	std::unique_ptr<OutlineRenderPass> m_outlineRenderPass;
	std::unique_ptr<LightProbeRenderPass> m_lightProbeRenderPass;

	// post-processing
	std::unique_ptr<AnamorphicPrefilterComputePass> m_anamorphicPrefilterComputePass;
	std::unique_ptr<AnamorphicDownsampleComputePass> m_anamorphicDownsampleComputePass;
	std::unique_ptr<AnamorphicUpsampleComputePass> m_anamorphicUpsampleComputePass;
	std::unique_ptr<FXAARenderPass> m_fxaaRenderPass;
	std::unique_ptr<SMAAEdgeDetectionRenderPass> m_smaaEdgeDetectionRenderPass;
	std::unique_ptr<SMAABlendWeightRenderPass> m_smaaBlendWeightRenderPass;
	std::unique_ptr<SMAABlendRenderPass> m_smaaBlendRenderPass;
	std::unique_ptr<SMAATemporalResolveRenderPass> m_smaaTemporalResolveRenderPass;
	std::unique_ptr<GodRayMaskComputePass> m_godRayMaskComputePass;
	std::unique_ptr<GodRayGenComputePass> m_godRayGenComputePass;
	std::unique_ptr<LuminanceGenComputePass> m_luminanceGenComputePass;
	std::unique_ptr<LuminanceAdaptionComputePass> m_luminanceAdaptionComputePass;
	std::unique_ptr<VelocityCorrectionComputePass> m_velocityCorrectionComputePass;
	std::unique_ptr<SimpleDofCocBlurComputePass> m_simpleDofCocBlurComputePass;
	std::unique_ptr<SimpleDofBlurComputePass> m_simpleDofBlurComputePass;
	std::unique_ptr<SimpleDofCompositeComputePass> m_simpleDofCompositeComputePass;
	std::unique_ptr<SpriteDofRenderPass> m_spriteDofRenderPass;
	std::unique_ptr<SpriteDofCompositeComputePass> m_spriteDofCompositeComputePass;
	std::unique_ptr<TileBasedDofDownsampleComputePass> m_tileBasedDofDownsampleComputePass;
	std::unique_ptr<TileBasedDofBlurComputePass> m_tileBasedDofBlurComputePass;
	std::unique_ptr<TileBasedDofFillComputePass> m_tileBasedDofFillComputePass;
	std::unique_ptr<TileBasedDofCompositeComputePass> m_tileBasedDofCompositeComputePass;
	std::unique_ptr<LuminanceHistogramComputePass> m_luminanceHistogramComputePass;
	std::unique_ptr<LuminanceHistogramReduceComputePass> m_luminanceHistogramReduceComputePass;
	std::unique_ptr<LuminanceHistogramAdaptionComputePass> m_luminanceHistogramAdaptionComputePass;
	std::unique_ptr<CocComputePass> m_cocComputePass;
	std::unique_ptr<CocTileMaxRenderPass> m_cocTileMaxRenderPass;
	std::unique_ptr<CocNeighborTileMaxRenderPass> m_cocNeighborTileMaxRenderPass;
	std::unique_ptr<VelocityTileMaxRenderPass> m_velocityTileMaxRenderPass;
	std::unique_ptr<VelocityNeighborTileMaxRenderPass> m_velocityNeighborTileMaxRenderPass;
	std::unique_ptr<LensFlareGenRenderPass> m_lensFlareGenRenderPass;
	std::unique_ptr<LensFlareBlurRenderPass> m_lensFlareBlurRenderPass;
	std::unique_ptr<BloomDownsampleComputePass> m_bloomDownsampleComputePass;
	std::unique_ptr<BloomUpsampleComputePass> m_bloomUpsampleComputePass;
	std::unique_ptr<SimplePostEffectsRenderPass> m_simplePostEffectsRenderPass;
	std::unique_ptr<ToneMapRenderPass> m_toneMapRenderPass;
	std::unique_ptr<AntiAliasingTonemapComputePass> m_antiAliasingTonemapComputePass;
	std::unique_ptr<AntiAliasingReverseTonemapComputePass> m_antiAliasingReverseTonemapComputePass;
#if PROFILING_ENABLED
	std::unique_ptr<MotionBlurRenderPass> m_motionBlurRenderPass;
#endif // PROFILING_ENABLED

	// debug
	std::unique_ptr<BoundingBoxRenderPass> m_boundingBoxRenderPass;

	unsigned int m_frame;
	bool m_currentLuminanceTexture;
	bool m_currentSmaaTexture;
	GLuint m_finishedTexture;
	GLuint m_ssaoTexture;
};