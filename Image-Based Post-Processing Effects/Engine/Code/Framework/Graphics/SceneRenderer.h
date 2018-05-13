#pragma once
#include <glad\glad.h>
#include "Uniform.h"

class Camera;
class Window;
class Scene;
class Mesh;
struct Effects;
struct Level;
class ShaderProgram;
struct RenderData;

class SceneRenderer
{
public:
	explicit SceneRenderer(std::shared_ptr<Window> _window);
	SceneRenderer(const SceneRenderer &) = delete;
	SceneRenderer(const SceneRenderer &&) = delete;
	SceneRenderer &operator= (const SceneRenderer &) = delete;
	SceneRenderer &operator= (const SceneRenderer &&) = delete;
	~SceneRenderer();
	void init();
	void render(const RenderData &_renderData, const Scene &_scene, const std::shared_ptr<Level> &_level, const Effects &_effects);
	void resize(const std::pair<unsigned int, unsigned int> &_resolution);
	GLuint getColorTexture() const;
	GLuint getDepthStencilTexture() const;
	GLuint getVelocityTexture() const;

private:
	// shaders
	std::shared_ptr<ShaderProgram> gBufferPassShader;
	std::shared_ptr<ShaderProgram> outlineShader;
	std::shared_ptr<ShaderProgram> environmentLightPassShader;
	std::shared_ptr<ShaderProgram> pointLightPassShader;
	std::shared_ptr<ShaderProgram> spotLightPassShader;
	std::shared_ptr<ShaderProgram> directionalLightShader;
	std::shared_ptr<ShaderProgram> skyboxShader;
	std::shared_ptr<ShaderProgram> transparencyShader;
	std::shared_ptr<ShaderProgram> ssaoShader;
	std::shared_ptr<ShaderProgram> ssaoBlurShader;
	std::shared_ptr<ShaderProgram> butterflyPrecomputeShader;
	std::shared_ptr<ShaderProgram> tildeH0kShader;
	std::shared_ptr<ShaderProgram> tildeHktShader;
	std::shared_ptr<ShaderProgram> butterflyComputeShader;
	std::shared_ptr<ShaderProgram> inversePermuteShader;
	std::shared_ptr<ShaderProgram> waterNormalShader;
	std::shared_ptr<ShaderProgram> waterShader;

	std::shared_ptr<Window> window;

	std::shared_ptr<Mesh> pointLightMesh;
	std::shared_ptr<Mesh> spotLightMesh;
	std::shared_ptr<Mesh> fullscreenTriangle;

	// water mesh
	glm::vec2 waterGridDimensions = glm::vec2(300);
	GLuint waterVBO;
	GLuint waterVAO;
	GLuint waterEBO;

	// g-buffer
	GLuint gBufferFBO;
	GLuint gAlbedoTexture;
	GLuint gNormalTexture;
	GLuint gMRASTexture; // metallic | roughness | ambient occlusion | shaded
	GLuint gLightColorTextures[2];
	GLuint gVelocityTexture;
	GLuint gDepthStencilTexture;

	std::size_t currentLightColorTexture;
	GLenum lightColorAttachments[2];
	

	// ssao fbo
	GLuint ssaoFbo;
	GLuint ssaoTextureA;
	GLuint ssaoTextureB;
	GLuint noiseTexture;

	// fft fbo
	GLuint fftFbo;
	GLuint tildeH0kTexture;
	GLuint tildeH0minusKTexture;
	GLuint tildeHktDxTexture;
	GLuint tildeHktDyTexture;
	GLuint tildeHktDzTexture;
	GLuint pingPongTextureA;
	GLuint pingPongTextureB;
	GLuint pingPongTextureC;

	// fft-twiddle indices fbo
	GLuint twiddleIndicesFbo;
	GLuint twiddleIndicesTexture;

	// displacement fbo
	GLuint waterFbo;
	GLuint waterDisplacementFoldingTexture;
	GLuint waterNormalTexture;

	// skybox uniforms
	Uniform<glm::mat4> uInverseModelViewProjectionB = Uniform<glm::mat4>("uInverseModelViewProjection");
	Uniform<GLint> uAlbedoMapB = Uniform<GLint>("uAlbedoMap");
	Uniform<glm::vec4> uColorB = Uniform<glm::vec4>("uColor");
	Uniform<GLboolean> uHasAlbedoMapB = Uniform<GLboolean>("uHasAlbedoMap");

	// gBufferPass uniforms
	Uniform<glm::mat4> uModelMatrixG = Uniform<glm::mat4>("uModelMatrix");
	Uniform<glm::mat4> uModelViewProjectionMatrixG = Uniform<glm::mat4>("uModelViewProjectionMatrix");
	Uniform<glm::mat4> uPrevTransformG = Uniform<glm::mat4>("uPrevTransform");
	Uniform<glm::vec4> uAtlasDataG = Uniform<glm::vec4>("uAtlasData");
	UniformMaterial uMaterialG = UniformMaterial("uMaterial");

	// outline uniforms
	Uniform<glm::mat4> uModelViewProjectionMatrixO = Uniform<glm::mat4>("uModelViewProjectionMatrix");
	Uniform<glm::vec4> uOutlineColorO = Uniform<glm::vec4>("uOutlineColor");

	// environmentLightPass uniforms
	Uniform<GLint> uAlbedoMapE = Uniform<GLint>("uAlbedoMap");
	Uniform<GLint> uNormalMapE = Uniform<GLint>("uNormalMap");
	Uniform<GLint> uMetallicRoughnessAoMapE = Uniform<GLint>("uMetallicRoughnessAoMap");
	Uniform<GLint> uDepthMapE = Uniform<GLint>("uDepthMap");
	Uniform<GLint> uSsaoMapE = Uniform<GLint>("uSsaoMap");
	Uniform<GLint> uPrevFrameE = Uniform<GLint>("uPrevFrame");
	Uniform<glm::mat4> uProjectionE = Uniform<glm::mat4>("uProjection");
	Uniform<glm::mat4> uInverseProjectionE = Uniform<glm::mat4>("uInverseProjection");
	Uniform<glm::mat4> uViewE = Uniform<glm::mat4>("uView");
	Uniform<glm::mat4> uInverseViewE = Uniform<glm::mat4>("uInverseView");
	Uniform<glm::mat4> uPrevViewProjectionE = Uniform<glm::mat4>("uPrevViewProjection");
	Uniform<glm::vec3> uCamPosE = Uniform<glm::vec3>("uCamPos");
	Uniform<GLint> uIrradianceMapE = Uniform<GLint>("uIrradianceMap");
	Uniform<GLint> uPrefilterMapE = Uniform<GLint>("uPrefilterMap");
	Uniform<GLint> uBrdfLUTE = Uniform<GLint>("uBrdfLUT");
	UniformDirectionalLight uDirectionalLightE = UniformDirectionalLight("uDirectionalLight");
	Uniform<GLboolean> uShadowsEnabledE = Uniform<GLboolean>("uShadowsEnabled");
	Uniform<GLboolean> uRenderDirectionalLightE = Uniform<GLboolean>("uRenderDirectionalLight");
	Uniform<GLboolean> uSsaoE = Uniform<GLboolean>("uSsao");
	Uniform<GLboolean> uUseSsrE = Uniform<GLboolean>("uUseSsr");

	// pointLightPass uniforms
	Uniform<glm::mat4> uModelViewProjectionP = Uniform<glm::mat4>("uModelViewProjection");
	Uniform<GLint> uAlbedoMapP = Uniform<GLint>("uAlbedoMap");
	Uniform<GLint> uNormalMapP = Uniform<GLint>("uNormalMap");
	Uniform<GLint> uMetallicRoughnessAoMapP = Uniform<GLint>("uMetallicRoughnessAoMap");
	Uniform<GLint> uDepthMapP = Uniform<GLint>("uDepthMap");
	UniformPointLight uPointLightP = UniformPointLight("uPointLight");
	Uniform<glm::mat4> uInverseProjectionP = Uniform<glm::mat4>("uInverseProjection");
	Uniform<glm::mat4> uInverseViewP = Uniform<glm::mat4>("uInverseView");
	Uniform<glm::vec3> uCamPosP = Uniform<glm::vec3>("uCamPos");
	Uniform<GLboolean> uShadowsEnabledP = Uniform<GLboolean>("uShadowsEnabled");
	Uniform<glm::vec2> uViewportSizeP = Uniform<glm::vec2>("uViewportSize");

	// spotLightPass uniforms
	Uniform<glm::mat4> uModelViewProjectionS = Uniform<glm::mat4>("uModelViewProjection");
	Uniform<GLint> uAlbedoMapS = Uniform<GLint>("uAlbedoMap");
	Uniform<GLint> uNormalMapS = Uniform<GLint>("uNormalMap");
	Uniform<GLint> uMetallicRoughnessAoMapS = Uniform<GLint>("uMetallicRoughnessAoMap");
	Uniform<GLint> uDepthMapS = Uniform<GLint>("uDepthMap");
	UniformSpotLight uSpotLightS = UniformSpotLight("uSpotLight");
	Uniform<glm::mat4> uInverseProjectionS = Uniform<glm::mat4>("uInverseProjection");
	Uniform<glm::mat4> uInverseViewS = Uniform<glm::mat4>("uInverseView");
	Uniform<glm::vec3> uCamPosS = Uniform<glm::vec3>("uCamPos");
	Uniform<GLboolean> uShadowsEnabledS = Uniform<GLboolean>("uShadowsEnabled");
	Uniform<glm::vec2> uViewportSizeS = Uniform<glm::vec2>("uViewportSize");

	// directionalLightPass uniforms
	Uniform<GLint> uAlbedoMapD = Uniform<GLint>("uAlbedoMap");
	Uniform<GLint> uNormalMapD = Uniform<GLint>("uNormalMap");
	Uniform<GLint> uMetallicRoughnessAoMapD = Uniform<GLint>("uMetallicRoughnessAoMap");
	Uniform<GLint> uDepthMapD = Uniform<GLint>("uDepthMap");
	UniformDirectionalLight uDirectionalLightD = UniformDirectionalLight("uDirectionalLight");
	Uniform<glm::mat4> uInverseViewD = Uniform<glm::mat4>("uInverseView");
	Uniform<glm::mat4> uInverseProjectionD = Uniform<glm::mat4>("uInverseProjection");
	Uniform<glm::vec3> uCamPosD = Uniform<glm::vec3>("uCamPos");
	Uniform<GLboolean> uShadowsEnabledD = Uniform<GLboolean>("uShadowsEnabled");

	// transparency uniforms
	Uniform<glm::mat4> uModelViewProjectionMatrixT = Uniform<glm::mat4>("uModelViewProjectionMatrix");
	Uniform<glm::mat4> uModelMatrixT = Uniform<glm::mat4>("uModelMatrix");
	Uniform<glm::vec4> uAtlasDataT = Uniform<glm::vec4>("uAtlasData");
	UniformMaterial uMaterialT = UniformMaterial("uMaterial");
	UniformDirectionalLight uDirectionalLightT = UniformDirectionalLight("uDirectionalLight");
	Uniform<GLboolean> uRenderDirectionalLightT = Uniform<GLboolean>("uRenderDirectionalLight");
	Uniform<glm::vec3> uCamPosT = Uniform<glm::vec3>("uCamPos");
	Uniform<GLboolean> uShadowsEnabledT = Uniform<GLboolean>("uShadowsEnabled");
	Uniform<GLint> uIrradianceMapT = Uniform<GLint>("uIrradianceMap");
	Uniform<GLint> uPrefilterMapT = Uniform<GLint>("uPrefilterMap");
	Uniform<GLint> uBrdfLUTT = Uniform<GLint>("uBrdfLUT");

	// ssao uniforms
	Uniform<GLint> uDepthTextureAO = Uniform<GLint>("uDepthTexture");
	Uniform<GLint> uNormalTextureAO = Uniform<GLint>("uNormalTexture");
	Uniform<GLint> uNoiseTextureAO = Uniform<GLint>("uNoiseTexture");
	Uniform<glm::mat4> uViewAO = Uniform<glm::mat4>("uView");
	Uniform<glm::mat4> uProjectionAO = Uniform<glm::mat4>("uProjection");
	Uniform<glm::mat4> uInverseProjectionAO = Uniform<glm::mat4>("uInverseProjection");
	std::vector<GLint> uSamplesAO;
	Uniform<GLint> uKernelSizeAO = Uniform<GLint>("uKernelSize");
	Uniform<GLfloat> uRadiusAO = Uniform<GLfloat>("uRadius");
	Uniform<GLfloat> uBiasAO = Uniform<GLfloat>("uBias");

	// ssao blur uniforms
	Uniform<GLint> uInputTextureAOB = Uniform<GLint>("uInputTexture");
	Uniform<GLint> uBlurSizeAOB = Uniform<GLint>("uBlurSize"); // size of noise texture

	// tildeh0k
	Uniform<GLint> uNoiseR0TextureH0 = Uniform<GLint>("uNoiseR0Texture");
	Uniform<GLint> uNoiseI0TextureH0 = Uniform<GLint>("uNoiseI0Texture");
	Uniform<GLint> uNoiseR1TextureH0 = Uniform<GLint>("uNoiseR1Texture");
	Uniform<GLint> uNoiseI1TextureH0 = Uniform<GLint>("uNoiseI1Texture");
	Uniform<GLint> uNH0 = Uniform<GLint>("uN");
	Uniform<GLint> uLH0 = Uniform<GLint>("uL");
	Uniform<GLfloat> uAH0 = Uniform<GLfloat>("uA");
	Uniform<glm::vec2> uWindDirectionH0 = Uniform<glm::vec2>("uWindDirection");
	Uniform<GLfloat> uWindSpeedH0 = Uniform<GLfloat>("uWindSpeed");
	Uniform<GLfloat> uWaveSuppressionExpH0 = Uniform<GLfloat>("uWaveSuppressionExp");

	// tildehkt
	Uniform<GLint> uTildeH0kTextureHT = Uniform<GLint>("uTildeH0kTexture");
	Uniform<GLint> uTildeH0minusKTextureHT = Uniform<GLint>("uTildeH0minusKTexture");
	Uniform<GLint> uNHT = Uniform<GLint>("uN");
	Uniform<GLint> uLHT = Uniform<GLint>("uL");
	Uniform<GLfloat> uTimeHT = Uniform<GLfloat>("uTime");

	// butterflyPrecompute
	std::vector<GLint> uJBP;
	Uniform<GLint> uNBP = Uniform<GLint>("uN");

	// butterflyCompute
	Uniform<GLint> uButterflyTextureBC = Uniform<GLint>("uButterflyTexture");
	Uniform<GLint> uInputXTextureBC = Uniform<GLint>("uInputXTexture");
	Uniform<GLint> uInputYTextureBC = Uniform<GLint>("uInputYTexture");
	Uniform<GLint> uInputZTextureBC = Uniform<GLint>("uInputZTexture");
	Uniform<GLint> uNBC = Uniform<GLint>("uN");
	Uniform<GLint> uStageBC = Uniform<GLint>("uStage");
	Uniform<GLint> uStagesBC = Uniform<GLint>("uStages");
	Uniform<GLint> uDirectionBC = Uniform<GLint>("uDirection");

	// inverse/permute
	Uniform<GLint> uInputXTextureIP = Uniform<GLint>("uInputXTexture");
	Uniform<GLint> uInputYTextureIP = Uniform<GLint>("uInputYTexture");
	Uniform<GLint> uInputZTextureIP = Uniform<GLint>("uInputZTexture");
	Uniform<GLint> uNIP = Uniform<GLint>("uN");
	Uniform<GLfloat> uChoppinessIP = Uniform<GLfloat>("uChoppiness");

	// water
	Uniform<GLint> uNormalTextureW = Uniform<GLint>("uNormalTexture");
	Uniform<GLint> uDisplacementTextureW = Uniform<GLint>("uDisplacementTexture");
	Uniform<GLint> uFoamTextureW = Uniform<GLint>("uFoamTexture");
	Uniform<GLint> uEnvironmentTextureW = Uniform<GLint>("uEnvironmentTexture");
	Uniform<glm::mat4> uProjectionW = Uniform<glm::mat4>("uProjection");
	Uniform<glm::mat4> uViewW = Uniform<glm::mat4>("uView");
	Uniform<glm::vec3> uCamPosW = Uniform<glm::vec3>("uCamPos"); 
	Uniform<glm::vec2> uTexCoordShiftW = Uniform<glm::vec2>("uTexCoordShift");
	Uniform<bool> uUseEnvironmentW = Uniform<bool>("uUseEnvironment");
	Uniform<float> uWaterLevelW = Uniform<float>("uWaterLevel");
	Uniform<glm::vec3> uLightDirW = Uniform<glm::vec3>("uLightDir");
	Uniform<glm::vec3> uLightColorW = Uniform<glm::vec3>("uLightColor");


	// water normal
	Uniform<GLint> uDisplacementTextureN = Uniform<GLint>("uDisplacementTexture");
	Uniform<GLfloat> uNormalStrengthN = Uniform<GLfloat>("uNormalStrength");

	void createFboAttachments(const std::pair<unsigned int, unsigned int> &_resolution);
	void createWaterAttachments();
	void createSsaoAttachments(const std::pair<unsigned int, unsigned int> &_resolution);
	void renderGeometry(const RenderData &_renderData, const Scene &_scene);
	void renderSkybox(const RenderData &_renderData, const std::shared_ptr<Level> &_level);
	void renderEnvironmentLight(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const glm::mat4 &_inverseView, const glm::mat4 &_inverseProjection, const Effects &_postEffects);
	void renderDirectionalLights(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const glm::mat4 &_inverseView, const glm::mat4 &_inverseProjection);
	void renderPointLights(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const glm::mat4 &_inverseView, const glm::mat4 &_inverseProjection);
	void renderSpotLights(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const glm::mat4 &_inverseView, const glm::mat4 &_inverseProjection);
	void renderTransparentGeometry(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const Scene &_scene);
	void renderOutlines(const RenderData &_renderData, const Scene &_scene);
	void renderCustomGeometry(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const Scene &_scene, bool _opaque);
	void renderSsaoTexture(const RenderData &_renderData, const glm::mat4 &_inverseProjection, const Effects &_postEffects);
	void precomputeFftTextures();
	void computeFft();
	void renderWater(const RenderData &_renderData, const std::shared_ptr<Level> &_level);
	void createWaterPlane(const glm::vec2 &_dimensions, GLuint &_VBO, GLuint &_VAO, GLuint &_EBO);
};

