#pragma once
#include <memory>
#include <glad\glad.h>
#include "GLRenderResources.h"
#include "RenderPass\Shadow\ShadowRenderPass.h"
#include "RenderPass\Geometry\GBufferRenderPass.h"
#include "RenderPass\Geometry\GBufferCustomRenderPass.h"
#include "RenderPass\SSAO\SSAOOriginalRenderPass.h"
#include "RenderPass\SSAO\SSAORenderPass.h"
#include "RenderPass\SSAO\HBAORenderPass.h"
#include "RenderPass\SSAO\GTAORenderPass.h"
#include "RenderPass\SSAO\GTAOSpatialDenoiseRenderPass.h"
#include "RenderPass\SSAO\GTAOTemporalDenoiseRenderPass.h"
#include "RenderPass\SSAO\SSAOBlurRenderPass.h"
#include "RenderPass\SSAO\SSAOBilateralBlurRenderPass.h"
#include "RenderPass\Geometry\SkyboxRenderPass.h"
#include "RenderPass\Lighting\AmbientLightRenderPass.h"
#include "RenderPass\Lighting\DirectionalLightRenderPass.h"
#include "RenderPass\Lighting\StencilRenderPass.h"
#include "RenderPass\Lighting\DeferredEnvironmentProbeRenderPass.h"
#include "RenderPass\Lighting\PointLightRenderPass.h"
#include "RenderPass\Lighting\SpotLightRenderPass.h"
#include "RenderPass\Geometry\ForwardRenderPass.h"
#include "RenderPass\Geometry\ForwardCustomRenderPass.h"
#include "RenderPass\Geometry\OutlineRenderPass.h"
#include "RenderPass\Geometry\LightProbeRenderPass.h"
#include "ComputePass/LensFlares/AnamorphicPrefilterComputePass.h"
#include "ComputePass/LensFlares/AnamorphicDownsampleComputePass.h"
#include "ComputePass/LensFlares/AnamorphicUpsampleComputePass.h"
#include "RenderPass/AntiAliasing/FXAARenderPass.h"
#include "RenderPass/AntiAliasing/SMAAEdgeDetectionRenderPass.h"
#include "RenderPass/AntiAliasing/SMAABlendWeightRenderPass.h"
#include "RenderPass/AntiAliasing/SMAABlendRenderPass.h"
#include "RenderPass/AntiAliasing/SMAATemporalResolveRenderPass.h"
#include "ComputePass/GodRays/GodRayMaskComputePass.h"
#include "ComputePass/GodRays/GodRayGenComputePass.h"
#include "ComputePass/Exposure/LuminanceGenComputePass.h"
#include "ComputePass/Exposure/LuminanceAdaptionComputePass.h"
#include "ComputePass/MotionBlur/VelocityCorrectionComputePass.h"
#include "ComputePass/DepthOfField/SimpleDofCocBlurComputePass.h"
#include "ComputePass/DepthOfField/SimpleDofBlurComputePass.h"
#include "ComputePass/DepthOfField/SimpleDofFillComputePass.h"
#include "ComputePass/DepthOfField/SimpleDofCompositeComputePass.h"
#include "RenderPass/DepthOfField/SpriteDofRenderPass.h"
#include "ComputePass/DepthOfField/SpriteDofCompositeComputePass.h"
#include "ComputePass/DepthOfField/SeperateDofDownsampleComputePass.h"
#include "ComputePass/DepthOfField/SeperateDofBlurComputePass.h"
#include "ComputePass/DepthOfField/SeperateDofFillComputePass.h"
#include "ComputePass/DepthOfField/SeperateDofCompositeComputePass.h"
#include "ComputePass/DepthOfField/CocComputePass.h"
#include "RenderPass/DepthOfField/CocTileMaxRenderPass.h"
#include "RenderPass/DepthOfField/CocNeighborTileMaxRenderPass.h"
#include "ComputePass/Exposure/LuminanceHistogramComputePass.h"
#include "ComputePass/Exposure/LuminanceHistogramReduceComputePass.h"
#include "ComputePass/Exposure/LuminanceHistogramAdaptionComputePass.h"
#include "RenderPass/MotionBlur/VelocityTileMaxRenderPass.h"
#include "RenderPass/MotionBlur/VelocityNeighborTileMaxRenderPass.h"
#include "RenderPass/LensFlares/LensFlareGenRenderPass.h"
#include "RenderPass/LensFlares/LensFlareBlurRenderPass.h"
#include "ComputePass/Bloom/BloomDownsampleComputePass.h"
#include "ComputePass/Bloom/BloomUpsampleComputePass.h"
#include "RenderPass/Misc/SimplePostEffectsRenderPass.h"
#include "RenderPass/Misc/ToneMapRenderPass.h"
#include "ComputePass/DepthOfField/CombinedDofTileMaxComputePass.h"
#include "ComputePass/DepthOfField/CombinedDofNeighborTileMaxComputePass.h"
#include "ComputePass/DepthOfField/SeperateDofTileMaxComputePass.h"
#include "ComputePass/AntiAliasing/AntiAliasingTonemapComputePass.h"
#include "ComputePass/AntiAliasing/AntiAliasingReverseTonemapComputePass.h"
#include "RenderPass/Debug/BoundingBoxRenderPass.h"

struct RenderResources;
class Scene;
struct Effects;
struct Level;
class ShaderProgram;
struct RenderData;
struct AxisAlignedBoundingBox;
struct Water;
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
class SeperateDofDownsampleComputePass;
class SeperateDofBlurComputePass;
class SeperateDofFillComputePass;
class SeperateDofCompositeComputePass;
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
class CombinedDofTileMaxComputePass;
class CombinedDofNeighborTileMaxComputePass;
class SeperateDofTileMaxComputePass;
class AntiAliasingTonemapComputePass;
class AntiAliasingReverseTonemapComputePass;

class GLRenderer
{
public:
	explicit GLRenderer();
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
	std::unique_ptr<SimpleDofFillComputePass> m_simpleDofFillComputePass;
	std::unique_ptr<SimpleDofCompositeComputePass> m_simpleDofCompositeComputePass;
	std::unique_ptr<SpriteDofRenderPass> m_spriteDofRenderPass;
	std::unique_ptr<SpriteDofCompositeComputePass> m_spriteDofCompositeComputePass;
	std::unique_ptr<SeperateDofDownsampleComputePass> m_seperateDofDownsampleComputePass;
	std::unique_ptr<SeperateDofBlurComputePass> m_seperateDofBlurComputePass;
	std::unique_ptr<SeperateDofFillComputePass> m_seperateDofFillComputePass;
	std::unique_ptr<SeperateDofCompositeComputePass> m_seperateDofCompositeComputePass;
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
	std::unique_ptr<CombinedDofTileMaxComputePass> m_combinedDofTileMaxComputePass;
	std::unique_ptr<CombinedDofNeighborTileMaxComputePass> m_combinedDofNeighborTileMaxComputePass;
	std::unique_ptr<SeperateDofTileMaxComputePass> m_seperateDofTileMaxComputePass;
	std::unique_ptr<AntiAliasingTonemapComputePass> m_antiAliasingTonemapComputePass;
	std::unique_ptr<AntiAliasingReverseTonemapComputePass> m_antiAliasingReverseTonemapComputePass;

	// debug
	std::unique_ptr<BoundingBoxRenderPass> m_boundingBoxRenderPass;

	unsigned int frame;
	bool currentLuminanceTexture;
	bool currentSmaaTexture;
	GLuint finishedTexture;
	GLuint ssaoTexture;
};