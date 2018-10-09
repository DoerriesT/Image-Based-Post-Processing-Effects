#pragma once
#include <glad\glad.h>
#include "Uniform.h"

class ShaderProgram;
class Camera;
class Mesh;
class Window;
struct Effects;
struct Level;
struct RenderData;
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

class PostProcessRenderer
{
public:
	explicit PostProcessRenderer(std::shared_ptr<Window> _window);
	PostProcessRenderer(const PostProcessRenderer &) = delete;
	PostProcessRenderer(const PostProcessRenderer &&) = delete;
	PostProcessRenderer &operator= (const PostProcessRenderer &) = delete;
	PostProcessRenderer &operator= (const PostProcessRenderer &&) = delete;
	~PostProcessRenderer();
	void init();
	void render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const Effects &_effects, GLuint _colorTexture, GLuint _depthTexture, GLuint _velocityTexture, const std::shared_ptr<Camera> &_camera);
	void resize(const std::pair<unsigned int, unsigned int> &_resolution);
	GLuint getFinishedTexture() const;

private:
	std::shared_ptr<Window> window;

	AnamorphicPrefilterComputePass *anamorphicPrefilterComputePass;
	AnamorphicDownsampleComputePass *anamorphicDownsampleComputePass;
	AnamorphicUpsampleComputePass *anamorphicUpsampleComputePass;
	FXAARenderPass *fxaaRenderPass;
	SMAAEdgeDetectionRenderPass *smaaEdgeDetectionRenderPass;
	SMAABlendWeightRenderPass *smaaBlendWeightRenderPass;
	SMAABlendRenderPass *smaaBlendRenderPass;
	SMAATemporalResolveRenderPass *smaaTemporalResolveRenderPass;
	GodRayMaskComputePass *godRayMaskComputePass;
	GodRayGenComputePass *godRayGenComputePass;
	LuminanceGenComputePass *luminanceGenComputePass;
	LuminanceAdaptionComputePass *luminanceAdaptionComputePass;
	VelocityCorrectionComputePass *velocityCorrectionComputePass;
	SimpleDofCocBlurComputePass *simpleDofCocBlurComputePass;
	SimpleDofBlurComputePass *simpleDofBlurComputePass;
	SimpleDofFillComputePass *simpleDofFillComputePass;
	SimpleDofCompositeComputePass *simpleDofCompositeComputePass;
	SpriteDofRenderPass *spriteDofRenderPass;
	SpriteDofCompositeComputePass *spriteDofCompositeComputePass;
	SeperateDofDownsampleComputePass *seperateDofDownsampleComputePass;
	SeperateDofBlurComputePass *seperateDofBlurComputePass;
	SeperateDofFillComputePass *seperateDofFillComputePass;
	SeperateDofCompositeComputePass *seperateDofCompositeComputePass;
	LuminanceHistogramComputePass *luminanceHistogramComputePass;
	LuminanceHistogramReduceComputePass *luminanceHistogramReduceComputePass;
	LuminanceHistogramAdaptionComputePass *luminanceHistogramAdaptionComputePass;
	CocComputePass *cocComputePass;
	CocTileMaxRenderPass *cocTileMaxRenderPass;
	CocNeighborTileMaxRenderPass *cocNeighborTileMaxRenderPass;
	VelocityTileMaxRenderPass *velocityTileMaxRenderPass;
	VelocityNeighborTileMaxRenderPass *velocityNeighborTileMaxRenderPass;
	LensFlareGenRenderPass *lensFlareGenRenderPass;
	LensFlareBlurRenderPass *lensFlareBlurRenderPass;
	BloomDownsampleComputePass *bloomDownsampleComputePass;
	BloomUpsampleComputePass *bloomUpsampleComputePass;
	SimplePostEffectsRenderPass *simplePostEffectsRenderPass;
	ToneMapRenderPass *toneMapRenderPass;
	CombinedDofTileMaxComputePass *combinedDofTileMaxComputePass;
	CombinedDofNeighborTileMaxComputePass *combinedDofNeighborTileMaxComputePass;
	SeperateDofTileMaxComputePass *seperateDofTileMaxComputePass;

	GLuint luminanceHistogramIntermediary;
	GLuint luminanceHistogram;

	GLuint finishedTexture;

	GLuint luminanceTempTexture;
	GLuint luminanceTexture[2];
	bool currentLuminanceTexture;

	GLuint fullResolutionFbo;
	GLuint fullResolutionTextureA;
	GLuint fullResolutionTextureB;
	GLuint fullResolutionHdrTexture;
	GLuint fullResolutionCocTexture;
	GLuint fullResolutionDofTexA;
	GLuint fullResolutionDofTexB;
	GLuint fullResolutionDofTexC;
	GLuint fullResolutionDofTexD;

	GLuint smaaFbo;
	GLuint fullResolutionSmaaEdgesTex;
	GLuint fullResolutionSmaaBlendTex;
	GLuint fullResolutionSmaaMLResultTex[2];
	GLuint fullResolutionSmaaResultTex;
	bool currentSmaaTexture;

	GLuint halfResolutionFbo;
	GLuint halfResolutionHdrTexA;
	GLuint halfResolutionHdrTexB;
	GLuint halfResolutionHdrTexC;
	GLuint halfResolutionCocTexA;
	GLuint halfResolutionCocTexB;
	GLuint halfResolutionDofTexA;
	GLuint halfResolutionDofTexB;
	GLuint halfResolutionDofTexC;
	GLuint halfResolutionDofTexD;
	GLuint halfResolutionDofDoubleTex;
	GLuint halfResolutionGodRayTexA;
	GLuint halfResolutionGodRayTexB;

	GLuint velocityFbo;
	GLuint velocityTexTmp;
	GLuint velocityMaxTex;
	GLuint velocityNeighborMaxTex;

	GLuint cocFbo;
	GLuint cocTexTmp;
	GLuint cocMaxTex;
	GLuint cocNeighborMaxTex;

	GLuint anamorphicPrefilter;
	GLuint anamorphicChain[6];

	void calculateLuminance(const Effects &_effects, GLuint _colorTexture);
	void calculateLuminanceHistogram(GLuint _colorTexture);
	void createFboAttachments(const std::pair<unsigned int, unsigned int> &_resolution);

};