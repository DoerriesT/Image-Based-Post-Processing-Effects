#pragma once
#include <glad\glad.h>
#include "Uniform.h"
#include "Ocean.h"
#include "VolumetricLighting.h"

class Camera;
class Window;
class Scene;
class Mesh;
struct Effects;
struct Level;
class ShaderProgram;
struct RenderData;
struct AxisAlignedBoundingBox;
struct Water;

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
	GLuint getBrdfLUT() const;

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
	std::shared_ptr<ShaderProgram> ssaoOriginalShader;
	std::shared_ptr<ShaderProgram> ssaoBlurShader;
	std::shared_ptr<ShaderProgram> hbaoShader;

	std::shared_ptr<Window> window;

	std::shared_ptr<Mesh> pointLightMesh;
	std::shared_ptr<Mesh> spotLightMesh;
	std::shared_ptr<Mesh> fullscreenTriangle;

	Ocean ocean;
	VolumetricLighting volumetricLighting;

	GLuint brdfLUT;

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
	GLuint noiseTexture2;

	// skybox uniforms
	Uniform<glm::mat4> uInverseModelViewProjectionB = Uniform<glm::mat4>("uInverseModelViewProjection");
	Uniform<glm::mat4> uCurrentToPrevTransformB = Uniform<glm::mat4>("uCurrentToPrevTransform");
	Uniform<GLint> uAlbedoMapB = Uniform<GLint>("uAlbedoMap");
	Uniform<glm::vec4> uColorB = Uniform<glm::vec4>("uColor");
	Uniform<GLboolean> uHasAlbedoMapB = Uniform<GLboolean>("uHasAlbedoMap");

	// gBufferPass uniforms
	Uniform<glm::mat3> uModelViewMatrixG = Uniform<glm::mat3>("uModelViewMatrix");
	Uniform<glm::mat4> uModelViewProjectionMatrixG = Uniform<glm::mat4>("uModelViewProjectionMatrix");
	Uniform<glm::mat4> uPrevTransformG = Uniform<glm::mat4>("uPrevTransform");
	Uniform<glm::vec4> uAtlasDataG = Uniform<glm::vec4>("uAtlasData");
	Uniform<glm::vec2> uVelG = Uniform<glm::vec2>("uVel");
	Uniform<GLfloat> uExposureTimeG = Uniform<GLfloat>("uExposureTime");
	Uniform<GLfloat> uMaxVelocityMagG = Uniform<GLfloat>("uMaxVelocityMag");
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
	Uniform<glm::mat4> uInverseViewE = Uniform<glm::mat4>("uInverseView");
	Uniform<glm::mat4> uPrevViewProjectionE = Uniform<glm::mat4>("uPrevViewProjection");
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
	Uniform<GLboolean> uShadowsEnabledD = Uniform<GLboolean>("uShadowsEnabled");

	// transparency uniforms
	Uniform<glm::mat4> uPrevTransformT = Uniform<glm::mat4>("uPrevTransform");
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
	Uniform<GLfloat> uStrengthAO = Uniform<GLfloat>("uStrength");

	// ssao original
	Uniform<GLint> uDepthTextureAOO = Uniform<GLint>("uDepthTexture");
	Uniform<GLint> uNoiseTextureAOO = Uniform<GLint>("uNoiseTexture");

	// ssao blur uniforms
	Uniform<GLint> uInputTextureAOB = Uniform<GLint>("uInputTexture");
	Uniform<GLint> uBlurSizeAOB = Uniform<GLint>("uBlurSize"); // size of noise texture

	// hbao
	Uniform<GLint> uDepthMapHBAO = Uniform<GLint>("uDepthMap");
	Uniform<GLint> uNoiseMapHBAO = Uniform<GLint>("uNoiseMap");
	Uniform<glm::vec2> uFocalLengthHBAO = Uniform<glm::vec2>("uFocalLength");
	Uniform<glm::mat4> uInverseProjectionHBAO = Uniform<glm::mat4>("uInverseProjection");
	Uniform<glm::vec2> uAOResHBAO = Uniform<glm::vec2>("uAORes");
	Uniform<glm::vec2> uInvAOResHBAO = Uniform<glm::vec2>("uInvAORes");
	Uniform<glm::vec2> uNoiseScaleHBAO = Uniform<glm::vec2>("uNoiseScale");
	Uniform<GLfloat> uStrengthHBAO = Uniform<GLfloat>("uStrength");
	Uniform<GLfloat> uRadiusHBAO = Uniform<GLfloat>("uRadius");
	Uniform<GLfloat> uRadius2HBAO = Uniform<GLfloat>("uRadius2");
	Uniform<GLfloat> uNegInvR2HBAO = Uniform<GLfloat>("uNegInvR2");
	Uniform<GLfloat> uTanBiasHBAO = Uniform<GLfloat>("uTanBias");
	Uniform<GLfloat> uMaxRadiusPixelsHBAO = Uniform<GLfloat>("uMaxRadiusPixels");
	Uniform<GLfloat> uNumDirectionsHBAO = Uniform<GLfloat>("uNumDirections");
	Uniform<GLfloat> uNumStepsHBAO = Uniform<GLfloat>("uNumSteps");

	void createFboAttachments(const std::pair<unsigned int, unsigned int> &_resolution);
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
	void createBrdfLUT();
	bool cullAABB(const glm::mat4 &_mvp, const AxisAlignedBoundingBox &_aabb);
};

