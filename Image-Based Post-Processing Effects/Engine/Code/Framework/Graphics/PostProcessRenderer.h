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
	void render(const Effects &_effects, const GLuint &_colorTexture, const GLuint &_depthTexture, const std::shared_ptr<Camera> &_camera);
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
	std::shared_ptr<Window> window;

	std::shared_ptr<Texture> lensColorTexture;
	std::shared_ptr<Texture> lensDirtTexture;
	std::shared_ptr<Texture> lensStarTexture;

	std::shared_ptr<Mesh> fullscreenTriangle;

	GLuint finishedTexture;

	GLuint fullResolutionFbo;
	GLuint fullResolutionTextureA;
	GLuint fullResolutionTextureB;

	GLuint halfResolutionFbo;
	GLuint halfResolutionHdrTexA;
	GLuint halfResolutionHdrTexB;
	GLuint halfResolutionHdrTexC;

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
	Uniform<glm::mat3> uLensStarMatrixH = Uniform<glm::mat3>("uLensStarMatrix");
	Uniform<GLboolean> uLensFlaresH = Uniform<GLboolean>("uLensFlares");
	Uniform<GLboolean> uBloomH = Uniform<GLboolean>("uBloom");
	Uniform<GLfloat> uBloomStrengthH = Uniform<GLfloat>("uBloomStrength");
	Uniform<GLfloat> uBloomDirtStrengthH = Uniform<GLfloat>("uBloomDirtStrength");
	Uniform<GLfloat> uExposureH = Uniform<GLfloat>("uExposure");
	
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
	Uniform<GLfloat> uHaloWidthLFG = Uniform<GLfloat>("uHaloWidth");
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

	void fxaa(const float &_subPixelAA, const float &_edgeThreshold, const float &_edgeThresholdMin);
	void singlePassEffects(const Effects &_effects);
	void downsample(const GLuint &_colorTexture);
	void upsample();
	void generateFlares(const Effects &_effects);
	void createFboAttachments(const std::pair<unsigned int, unsigned int> &_resolution);
	
};