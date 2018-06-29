#pragma once
#include <glad\glad.h>
#include "Uniform.h"

class ShaderProgram;
class Camera;
class Mesh;
class Window;
struct Effects;

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
	void render(const Effects &_effects, GLuint _colorTexture, GLuint _depthTexture, GLuint _velocityTexture, const std::shared_ptr<Camera> &_camera);
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
	std::shared_ptr<ShaderProgram> dofBlurShader;
	std::shared_ptr<ShaderProgram> dofFillShader;
	std::shared_ptr<ShaderProgram> dofCompositeShader;
	std::shared_ptr<ShaderProgram> luminanceGenShader;
	std::shared_ptr<ShaderProgram> luminanceAdaptionShader;
	std::shared_ptr<Window> window;

	std::shared_ptr<Texture> lensColorTexture;
	std::shared_ptr<Texture> lensDirtTexture;
	std::shared_ptr<Texture> lensStarTexture;

	std::shared_ptr<Mesh> fullscreenTriangle;

	GLuint finishedTexture;

	GLuint luminanceTempTexture;
	GLuint luminanceTexture[2];
	bool currentLuminanceTexture;

	GLuint fullResolutionFbo;
	GLuint fullResolutionTextureA;
	GLuint fullResolutionTextureB;
	GLuint fullResolutionHdrTexture;
	GLuint fullResolutionCocTexture;

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

	// single pass effects uniforms
	Uniform<GLint> uScreenTextureS = Uniform<GLint>("uScreenTexture");
	Uniform<GLfloat> uTimeS = Uniform<GLfloat>("uTime");
	Uniform<GLfloat> uFilmGrainStrengthS = Uniform<GLfloat>("uFilmGrainStrength");
	Uniform<GLboolean> uVignetteS = Uniform<GLboolean>("uVignette");
	Uniform<GLboolean> uFilmGrainS = Uniform<GLboolean>("uFilmGrain");
	Uniform<GLboolean> uChromaticAberrationS = Uniform<GLboolean>("uChromaticAberration");
	Uniform<GLfloat> uChromAbOffsetMultiplierS = Uniform<GLfloat>("uChromAbOffsetMultiplier");

	// hdr uniform
	Uniform<GLint> uScreenTextureH = Uniform<GLint>("uScreenTexture"); // full resolution source texture
	Uniform<GLint> uBloomTextureH = Uniform<GLint>("uBloomTexture");
	Uniform<GLint> uLensFlareTexH = Uniform<GLint>("uLensFlareTex"); // input from the blur stage
	Uniform<GLint> uLensDirtTexH = Uniform<GLint>("uLensDirtTex"); // full resolution dirt texture
	Uniform<GLint> uLensStarTexH = Uniform<GLint>("uLensStarTex"); // diffraction starburst texture
	Uniform<GLfloat> uStarburstOffsetH = Uniform<GLfloat>("uStarburstOffset");
	Uniform<GLboolean> uLensFlaresH = Uniform<GLboolean>("uLensFlares");
	Uniform<GLboolean> uBloomH = Uniform<GLboolean>("uBloom");
	Uniform<GLint> uMotionBlurH = Uniform<GLint>("uMotionBlur");
	Uniform<GLfloat> uBloomStrengthH = Uniform<GLfloat>("uBloomStrength");
	Uniform<GLfloat> uBloomDirtStrengthH = Uniform<GLfloat>("uBloomDirtStrength");
	Uniform<GLfloat> uExposureH = Uniform<GLfloat>("uExposure");
	Uniform<GLint> uVelocityTextureH = Uniform<GLint>("uVelocityTexture");
	Uniform<GLint> uVelocityNeighborMaxTextureH = Uniform<GLint>("uVelocityNeighborMaxTexture");
	Uniform<GLint> uDepthTextureH = Uniform<GLint>("uDepthTexture");
	Uniform<GLfloat> uVelocityScaleH = Uniform<GLfloat>("uVelocityScale");
	Uniform<GLint> uLuminanceTextureH = Uniform<GLint>("uLuminanceTexture");

	// fxaa uniforms
	Uniform<GLint> uScreenTextureF = Uniform<GLint>("uScreenTexture");
	Uniform<glm::vec2> uInverseResolutionF = Uniform<glm::vec2>("uInverseResolution");
	Uniform<GLfloat> uSubPixelAAF = Uniform<GLfloat>("uSubPixelAA");
	Uniform<GLfloat> uEdgeThresholdF = Uniform<GLfloat>("uEdgeThreshold");
	Uniform<GLfloat> uEdgeThresholdMinF = Uniform<GLfloat>("uEdgeThresholdMin");

	// lens flare gen uniforms
	Uniform<GLint> uInputTexLFG = Uniform<GLint>("uInputTex");
	Uniform<GLint> uLensColorLFG = Uniform<GLint>("uLensColor");
	Uniform<GLint> uGhostsLFG = Uniform<GLint>("uGhosts");
	Uniform<GLfloat> uGhostDispersalLFG = Uniform<GLfloat>("uGhostDispersal");
	Uniform<GLfloat> uHaloRadiusLFG = Uniform<GLfloat>("uHaloRadius");
	Uniform<GLfloat> uDistortionLFG = Uniform<GLfloat>("uDistortion");
	Uniform<glm::vec4> uScaleLFG = Uniform<glm::vec4>("uScale");
	Uniform<glm::vec4> uBiasLFG = Uniform<glm::vec4>("uBias");

	// lens flare blur shader
	Uniform<GLint> uInputTexLFB = Uniform<GLint>("uInputTex");
	Uniform<GLboolean> uDirectionLFB = Uniform<GLboolean>("uDirection");

	// downsample
	Uniform<GLint> uColorTextureDS2 = Uniform<GLint>("uColorTexture");

	// bloom upscale
	Uniform<GLint> uUpscaleTextureBU = Uniform<GLint>("uUpscaleTexture");
	Uniform<GLint> uPreviousBlurredTextureBU = Uniform<GLint>("uPreviousBlurredTexture");
	Uniform<GLboolean> uAddPreviousBU = Uniform<GLboolean>("uAddPrevious");
	Uniform<glm::vec2> uRadiusBU = Uniform<glm::vec2>("uRadius");

	// velocity tile max
	Uniform<GLint> uVelocityTextureVTM = Uniform<GLint>("uVelocityTexture");
	Uniform<GLboolean> uDirectionVTM = Uniform<GLboolean>("uDirection");
	Uniform<GLint> uTileSizeVTM = Uniform<GLint>("uTileSize");

	// velocity neighbor tile max
	Uniform<GLint> uVelocityTextureVNTM = Uniform<GLint>("uVelocityTexture");

	// coc
	Uniform<GLint> uDepthTextureCOC = Uniform<GLint>("uDepthTexture");
	Uniform<GLfloat> uFocusDistanceCOC = Uniform<GLfloat>("uFocusDistance");
	Uniform<GLfloat> uFocalLengthCOC = Uniform<GLfloat>("uFocalLength");

	// coc blur
	Uniform<GLint> uCocTextureCOCB = Uniform<GLint>("uCocTexture");
	Uniform<GLboolean> uDirectionCOCB = Uniform<GLboolean>("uDirection");

	// dof blur
	Uniform<GLint> uColorTextureDOFB = Uniform<GLint>("uColorTexture");
	Uniform<GLint> uCocTextureDOFB = Uniform<GLint>("uCocTexture");
	std::vector<GLint> uSampleCoordsDOFB;
	Uniform<GLfloat> uBokehScaleDOFB = Uniform<GLfloat>("uBokehScale");

	// dof fill
	Uniform<GLint> uColorNearTextureDOFF = Uniform<GLint>("uColorNearTexture");
	Uniform<GLint> uColorFarTextureDOFF = Uniform<GLint>("uColorFarTexture");
	std::vector<GLint> uSampleCoordsDOFF;
	Uniform<GLfloat> uBokehScaleDOFF = Uniform<GLfloat>("uBokehScale");

	// dof composite
	Uniform<GLint> uNearTextureDOFC = Uniform<GLint>("uNearTexture");
	Uniform<GLint> uFarTextureDOFC = Uniform<GLint>("uFarTexture");
	Uniform<GLint> uColorTextureDOFC = Uniform<GLint>("uColorTexture");

	// luminance gen
	Uniform<GLint> uColorTextureLG = Uniform<GLint>("uColorTexture");

	// luminance adaption
	Uniform<GLint> uPrevLuminanceTextureLA = Uniform<GLint>("uPrevLuminanceTexture");
	Uniform<GLint> uCurrentLuminanceTextureLA = Uniform<GLint>("uCurrentLuminanceTexture");
	Uniform<GLfloat> uTimeDeltaLA = Uniform<GLfloat>("uTimeDelta");
	Uniform<GLfloat> uTauLA = Uniform<GLfloat>("uTau");


	void fxaa(float _subPixelAA, float _edgeThreshold, float _edgeThresholdMin);
	void singlePassEffects(const Effects &_effects);
	void downsample(GLuint _colorTexture);
	void upsample();
	void generateFlares(const Effects &_effects);
	void calculateCoc(GLuint _depthTexture);
	void simpleDepthOfField(GLuint _colorTexture, GLuint _depthTexture);
	void calculateLuminance(GLuint _colorTexture);
	void createFboAttachments(const std::pair<unsigned int, unsigned int> &_resolution);

};