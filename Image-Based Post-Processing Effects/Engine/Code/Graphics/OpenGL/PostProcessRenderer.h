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
	std::shared_ptr<ShaderProgram> singlePassEffectsShader;
	std::shared_ptr<ShaderProgram> hdrShader;
	std::shared_ptr<ShaderProgram> lensFlareGenShader;
	std::shared_ptr<ShaderProgram> lensFlareBlurShader;
	std::shared_ptr<ShaderProgram> downsampleShader;
	std::shared_ptr<ShaderProgram> upsampleShader;
	std::shared_ptr<ShaderProgram> velocityTileMaxShader;
	std::shared_ptr<ShaderProgram> velocityNeighborTileMaxShader;
	std::shared_ptr<ShaderProgram> cocShader;
	std::shared_ptr<ShaderProgram> cocTileMaxShader;
	std::shared_ptr<ShaderProgram> cocNeighborTileMaxShader;
	std::shared_ptr<ShaderProgram> dofSeperateDownsampleShader;
	std::shared_ptr<ShaderProgram> dofSeperateBlurShader;
	std::shared_ptr<ShaderProgram> dofSeperateFillShader;
	std::shared_ptr<ShaderProgram> dofSeperateCompositeShader;
	std::shared_ptr<ShaderProgram> luminanceHistogramShader;
	std::shared_ptr<ShaderProgram> luminanceHistogramReduceShader;
	std::shared_ptr<ShaderProgram> luminanceHistogramAdaptionShader;
	std::shared_ptr<Window> window;

	std::shared_ptr<Texture> lensColorTexture;
	std::shared_ptr<Texture> lensDirtTexture;
	std::shared_ptr<Texture> lensStarTexture;

	std::shared_ptr<Mesh> fullscreenTriangle;

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

	GLuint resolution4Fbo;
	GLuint resolution4HdrTexA;
	GLuint resolution4HdrTexB;

	GLuint resolution8Fbo;
	GLuint resolution8HdrTexA;
	GLuint resolution8HdrTexB;

	GLuint resolution16Fbo;
	GLuint resolution16HdrTexA;
	GLuint resolution16HdrTexB;

	GLuint resolution32Fbo;
	GLuint resolution32HdrTexA;
	GLuint resolution32HdrTexB;

	GLuint resolution64Fbo;
	GLuint resolution64HdrTexA;
	GLuint resolution64HdrTexB;

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

	// single pass effects uniforms
	Uniform<GLfloat> uTimeS = Uniform<GLfloat>("uTime");
	Uniform<GLfloat> uFilmGrainStrengthS = Uniform<GLfloat>("uFilmGrainStrength");
	Uniform<GLboolean> uVignetteS = Uniform<GLboolean>("uVignette");
	Uniform<GLboolean> uFilmGrainS = Uniform<GLboolean>("uFilmGrain");
	Uniform<GLboolean> uChromaticAberrationS = Uniform<GLboolean>("uChromaticAberration");
	Uniform<GLfloat> uChromAbOffsetMultiplierS = Uniform<GLfloat>("uChromAbOffsetMultiplier");

	// hdr uniform
	Uniform<GLfloat> uStarburstOffsetH = Uniform<GLfloat>("uStarburstOffset");
	Uniform<GLfloat> uBloomStrengthH = Uniform<GLfloat>("uBloomStrength");
	Uniform<GLfloat> uLensDirtStrengthH = Uniform<GLfloat>("uLensDirtStrength");
	Uniform<GLfloat> uExposureH = Uniform<GLfloat>("uExposure");
	Uniform<glm::vec3> uAnamorphicFlareColorH = Uniform<glm::vec3>("uAnamorphicFlareColor");

	// lens flare gen uniforms
	Uniform<GLint> uGhostsLFG = Uniform<GLint>("uGhosts");
	Uniform<GLfloat> uGhostDispersalLFG = Uniform<GLfloat>("uGhostDispersal");
	Uniform<GLfloat> uHaloRadiusLFG = Uniform<GLfloat>("uHaloRadius");
	Uniform<GLfloat> uDistortionLFG = Uniform<GLfloat>("uDistortion");
	Uniform<glm::vec4> uScaleLFG = Uniform<glm::vec4>("uScale");
	Uniform<glm::vec4> uBiasLFG = Uniform<glm::vec4>("uBias");

	// lens flare blur shader
	Uniform<GLboolean> uDirectionLFB = Uniform<GLboolean>("uDirection");

	// bloom upscale
	Uniform<GLboolean> uAddPreviousBU = Uniform<GLboolean>("uAddPrevious");
	Uniform<glm::vec2> uRadiusBU = Uniform<glm::vec2>("uRadius");

	// velocity tile max
	Uniform<GLboolean> uDirectionVTM = Uniform<GLboolean>("uDirection");
	Uniform<GLint> uTileSizeVTM = Uniform<GLint>("uTileSize");

	// coc
	Uniform<GLfloat> uFocalLengthCOC = Uniform<GLfloat>("uFocalLength");
	Uniform<GLfloat> uApertureSizeCOC = Uniform<GLfloat>("uApertureSize");
	Uniform<glm::vec2> uNearFarCOC = Uniform<glm::vec2>("uNearFar");

	// coc tile max
	Uniform<GLboolean> uDirectionCOCTM = Uniform<GLboolean>("uDirection");
	Uniform<GLint> uTileSizeCOCTM = Uniform<GLint>("uTileSize");

	// dof seperate blur
	std::vector<GLint> uSampleCoordsSDOFB;

	// dof seperate fill
	std::vector<GLint> uSampleCoordsSDOFF;

	// dof combined blur
	std::vector<GLint> uSampleCoordsCDOFB;

	// dof sprite
	Uniform<GLint> uWidthDOF = Uniform<GLint>("uWidth");
	Uniform<GLint> uHeightDOF = Uniform<GLint>("uHeight");

	// histogram
	Uniform<glm::vec2> uParamsLH = Uniform<glm::vec2>("uParams"); // multiply / add

	// histogram reduce
	Uniform<GLint> uLinesLHR = Uniform<GLint>("uLines");

	// histogram adaption
	Uniform<GLfloat> uTimeDeltaLHA = Uniform<GLfloat>("uTimeDelta");
	Uniform<GLfloat> uTauLHA = Uniform<GLfloat>("uTau");
	Uniform<glm::vec2> uParamsLHA = Uniform<glm::vec2>("uParams"); // multiply / add

	void fxaa(const Effects &_effects);
	void smaa(const Effects &_effects, GLuint _colorTexture, GLuint _velocityTexture, bool _temporalAA);
	void singlePassEffects(const Effects &_effects);
	void downsample(GLuint _colorTexture);
	void upsample();
	void generateFlares(const Effects &_effects);
	void anamorphicFlares(const Effects &_effects, GLuint _colorTexture);
	void calculateCoc(GLuint _depthTexture);
	void calculateCocTileTexture();
	void simpleDepthOfField(GLuint _colorTexture, GLuint _depthTexture);
	void tileBasedSeperateFieldDepthOfField(GLuint _colorTexture);
	void spriteBasedDepthOfField(GLuint _colorTexture, GLuint _depthTexture);
	void godRays(const Effects &_effects, const glm::vec2 &_sunpos, GLuint _colorTexture, GLuint _depthTexture);
	void calculateLuminance(const Effects &_effects, GLuint _colorTexture);
	void calculateLuminanceHistogram(GLuint _colorTexture);
	void correctVelocities(const RenderData &_renderData, GLuint _velocityTexture, GLuint _depthTexture);
	void createFboAttachments(const std::pair<unsigned int, unsigned int> &_resolution);

};