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
	std::shared_ptr<ShaderProgram> fxaaShader;
	std::shared_ptr<ShaderProgram> lensFlareGenShader;
	std::shared_ptr<ShaderProgram> lensFlareBlurShader;
	std::shared_ptr<ShaderProgram> downsampleShader;
	std::shared_ptr<ShaderProgram> upsampleShader;
	std::shared_ptr<ShaderProgram> velocityTileMaxShader;
	std::shared_ptr<ShaderProgram> velocityNeighborTileMaxShader;
	std::shared_ptr<ShaderProgram> cocShader;
	std::shared_ptr<ShaderProgram> cocBlurShader;
	std::shared_ptr<ShaderProgram> cocTileMaxShader;
	std::shared_ptr<ShaderProgram> cocNeighborTileMaxShader;
	std::shared_ptr<ShaderProgram> dofBlurShader;
	std::shared_ptr<ShaderProgram> dofFillShader;
	std::shared_ptr<ShaderProgram> dofCompositeShader;
	std::shared_ptr<ShaderProgram> dofSeperateBlurShader;
	std::shared_ptr<ShaderProgram> dofSeperateFillShader;
	std::shared_ptr<ShaderProgram> dofSeperateCompositeShader;
	std::shared_ptr<ShaderProgram> dofCombinedBlurShader;
	std::shared_ptr<ShaderProgram> dofSpriteShader;
	std::shared_ptr<ShaderProgram> dofSpriteComposeShader;
	std::shared_ptr<ShaderProgram> luminanceGenShader;
	std::shared_ptr<ShaderProgram> luminanceAdaptionShader;
	std::shared_ptr<ShaderProgram> godRayMaskShader;
	std::shared_ptr<ShaderProgram> godRayGenShader;
	std::shared_ptr<ShaderProgram> smaaEdgeDetectionShader;
	std::shared_ptr<ShaderProgram> smaaBlendingWeightCalculationShader;
	std::shared_ptr<ShaderProgram> smaaNeighborhoodBlendingShader;
	std::shared_ptr<ShaderProgram> smaaResolveShader;
	std::shared_ptr<Window> window;

	std::shared_ptr<Texture> lensColorTexture;
	std::shared_ptr<Texture> lensDirtTexture;
	std::shared_ptr<Texture> lensStarTexture;

	std::shared_ptr<Mesh> fullscreenTriangle;

	GLuint spriteVAO;
	GLuint spriteVBO;
	GLuint spriteEBO;

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

	// fxaa uniforms
	Uniform<glm::vec2> uInverseResolutionF = Uniform<glm::vec2>("uInverseResolution");
	Uniform<GLfloat> uSubPixelAAF = Uniform<GLfloat>("uSubPixelAA");
	Uniform<GLfloat> uEdgeThresholdF = Uniform<GLfloat>("uEdgeThreshold");
	Uniform<GLfloat> uEdgeThresholdMinF = Uniform<GLfloat>("uEdgeThresholdMin");

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

	// coc blur
	Uniform<GLboolean> uDirectionCOCB = Uniform<GLboolean>("uDirection");

	// coc tile max
	Uniform<GLboolean> uDirectionCOCTM = Uniform<GLboolean>("uDirection");
	Uniform<GLint> uTileSizeCOCTM = Uniform<GLint>("uTileSize");

	// dof blur
	std::vector<GLint> uSampleCoordsDOFB;

	// dof fill
	std::vector<GLint> uSampleCoordsDOFF;

	// dof seperate blur
	std::vector<GLint> uSampleCoordsSDOFB;

	// dof seperate fill
	std::vector<GLint> uSampleCoordsSDOFF;

	// dof combined blur
	std::vector<GLint> uSampleCoordsCDOFB;

	// dof sprite
	Uniform<GLint> uWidthDOF = Uniform<GLint>("uWidth");
	Uniform<GLint> uHeightDOF = Uniform<GLint>("uHeight");

	// luminance adaption
	Uniform<GLfloat> uTimeDeltaLA = Uniform<GLfloat>("uTimeDelta");
	Uniform<GLfloat> uTauLA = Uniform<GLfloat>("uTau");

	// god ray gen
	Uniform<glm::vec2> uSunPosGR = Uniform<glm::vec2>("uSunPos");

	// smaa edges
	Uniform<glm::vec4> uResolutionSMAAE = Uniform<glm::vec4>("uResolution");

	// smaa blend
	Uniform<glm::vec4> uResolutionSMAAB = Uniform<glm::vec4>("uResolution");
	Uniform<GLboolean> uTemporalSampleSMAAB = Uniform<GLboolean>("uTemporalSample");
	Uniform<GLboolean> uTemporalAASMAAB = Uniform<GLboolean>("uTemporalAA");

	// smaa combine
	Uniform<glm::vec4> uResolutionSMAAC = Uniform<glm::vec4>("uResolution");

	// smaa resolve
	Uniform<glm::vec4> uResolutionSMAAR = Uniform<glm::vec4>("uResolution");

	void fxaa(float _subPixelAA, float _edgeThreshold, float _edgeThresholdMin);
	void smaa(GLuint _colorTexture, GLuint _velocityTexture, bool _temporalAA);
	void singlePassEffects(const Effects &_effects);
	void downsample(GLuint _colorTexture);
	void upsample();
	void generateFlares(const Effects &_effects);
	void calculateCoc(GLuint _depthTexture);
	void calculateCocTileTexture();
	void simpleDepthOfField(GLuint _colorTexture, GLuint _depthTexture);
	void tileBasedSeperateFieldDepthOfField(GLuint _colorTexture);
	void tileBasedCombinedFieldDepthOfField(GLuint _colorTexture, GLuint _depthTexture);
	void spriteBasedDepthOfField(GLuint _colorTexture, GLuint _depthTexture);
	void godRays(const glm::vec2 &_sunpos, GLuint _colorTexture, GLuint _depthTexture);
	void calculateLuminance(GLuint _colorTexture);
	void createFboAttachments(const std::pair<unsigned int, unsigned int> &_resolution);

};