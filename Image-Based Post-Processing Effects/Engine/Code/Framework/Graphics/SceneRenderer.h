#pragma once
#include <glad\glad.h>
#include "Uniform.h"

class Camera;
class Window;
class Scene;
class Mesh;
class TileRing;
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
	std::shared_ptr<ShaderProgram> butterflyPrecomputeShader;
	std::shared_ptr<ShaderProgram> tildeH0kShader;
	std::shared_ptr<ShaderProgram> tildeHktShader;
	std::shared_ptr<ShaderProgram> butterflyComputeShader;
	std::shared_ptr<ShaderProgram> inversePermuteShader;
	std::shared_ptr<ShaderProgram> waterNormalShader;
	std::shared_ptr<ShaderProgram> waterShader;
	std::shared_ptr<ShaderProgram> waterTessShader;
	std::shared_ptr<ShaderProgram> lightVolumeShader;
	std::shared_ptr<ShaderProgram> phaseLUTShader;

	std::shared_ptr<ShaderProgram> butterflyPrecomputeCompShader;
	std::shared_ptr<ShaderProgram> tildeH0kCompShader;
	std::shared_ptr<ShaderProgram> tildeHktCompShader;
	std::shared_ptr<ShaderProgram> butterflyComputeCompShader;
	std::shared_ptr<ShaderProgram> inversePermuteCompShader;
	std::shared_ptr<ShaderProgram> waterNormalCompShader;

	std::shared_ptr<Window> window;

	std::shared_ptr<Mesh> pointLightMesh;
	std::shared_ptr<Mesh> spotLightMesh;
	std::shared_ptr<Mesh> fullscreenTriangle;

	TileRing *tileRings[6];

	GLuint brdfLUT;

	GLuint phaseLUT;

	// water mesh
	glm::vec2 waterGridDimensions = glm::vec2(300);
	GLuint waterVBO;
	GLuint waterVAO;
	GLuint waterEBO;

	// light volume mesh
	GLuint lightVolumeVAO;
	GLuint lightVolumeVBO;
	GLuint lightVolumeEBO;

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

	// light volume
	GLuint lightVolumeTexture;

	// fft-twiddle indices fbo
	GLuint twiddleIndicesFbo;
	GLuint twiddleIndicesTexture;

	// displacement fbo
	GLuint waterFbo;
	GLuint waterDisplacementFoldingTexture;
	GLuint waterNormalTexture;

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

	// tildeh0k
	Uniform<GLint> uNoiseR0TextureH0 = Uniform<GLint>("uNoiseR0Texture");
	Uniform<GLint> uNoiseI0TextureH0 = Uniform<GLint>("uNoiseI0Texture");
	Uniform<GLint> uNoiseR1TextureH0 = Uniform<GLint>("uNoiseR1Texture");
	Uniform<GLint> uNoiseI1TextureH0 = Uniform<GLint>("uNoiseI1Texture");
	Uniform<GLint> uSimulationResolutionH0 = Uniform<GLint>("uN");
	Uniform<GLint> uWorldSizeH0 = Uniform<GLint>("uL");
	Uniform<GLfloat> uWaveAmplitudeH0 = Uniform<GLfloat>("uA");
	Uniform<glm::vec2> uWindDirectionH0 = Uniform<glm::vec2>("uWindDirection");
	Uniform<GLfloat> uWindSpeedH0 = Uniform<GLfloat>("uWindSpeed");
	Uniform<GLfloat> uWaveSuppressionExpH0 = Uniform<GLfloat>("uWaveSuppressionExp");

	// tildehkt
	Uniform<GLint> uTildeH0kTextureHT = Uniform<GLint>("uTildeH0kTexture");
	Uniform<GLint> uTildeH0minusKTextureHT = Uniform<GLint>("uTildeH0minusKTexture");
	Uniform<GLint> uSimulationResolutionHT = Uniform<GLint>("uN");
	Uniform<GLint> uWorldSizeHT = Uniform<GLint>("uL");
	Uniform<GLfloat> uTimeHT = Uniform<GLfloat>("uTime");

	// butterflyPrecompute
	std::vector<GLint> uJBP;
	Uniform<GLint> uSimulationResolutionBP = Uniform<GLint>("uN");

	// butterflyCompute
	Uniform<GLint> uButterflyTextureBC = Uniform<GLint>("uButterflyTexture");
	Uniform<GLint> uInputXTextureBC = Uniform<GLint>("uInputXTexture");
	Uniform<GLint> uInputYTextureBC = Uniform<GLint>("uInputYTexture");
	Uniform<GLint> uInputZTextureBC = Uniform<GLint>("uInputZTexture");
	Uniform<GLint> uSimulationResolutionBC = Uniform<GLint>("uN");
	Uniform<GLint> uStageBC = Uniform<GLint>("uStage");
	Uniform<GLint> uStagesBC = Uniform<GLint>("uStages");
	Uniform<GLint> uDirectionBC = Uniform<GLint>("uDirection");

	// inverse/permute
	Uniform<GLint> uInputXTextureIP = Uniform<GLint>("uInputXTexture");
	Uniform<GLint> uInputYTextureIP = Uniform<GLint>("uInputYTexture");
	Uniform<GLint> uInputZTextureIP = Uniform<GLint>("uInputZTexture");
	Uniform<GLint> uSimulationResolutionIP = Uniform<GLint>("uN");
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

	// water tesselated
	Uniform<GLint> uNormalTextureWT = Uniform<GLint>("uNormalTexture");
	Uniform<GLint> uDisplacementTextureWT = Uniform<GLint>("uDisplacementTexture");
	Uniform<GLint> uFoamTextureWT = Uniform<GLint>("uFoamTexture");
	Uniform<GLint> uEnvironmentTextureWT = Uniform<GLint>("uEnvironmentTexture");
	Uniform<glm::mat4> uViewProjectionWT = Uniform<glm::mat4>("uViewProjection");
	Uniform<glm::mat4> uProjectionWT = Uniform<glm::mat4>("uProjection");
	Uniform<glm::mat4> uViewWT = Uniform<glm::mat4>("uView");
	Uniform<glm::vec3> uCamPosWT = Uniform<glm::vec3>("uCamPos");
	Uniform<glm::vec2> uTexCoordShiftWT = Uniform<glm::vec2>("uTexCoordShift");
	Uniform<bool> uUseEnvironmentWT = Uniform<bool>("uUseEnvironment");
	Uniform<float> uWaterLevelWT = Uniform<float>("uVerticalDisplacement");
	Uniform<glm::vec3> uLightDirWT = Uniform<glm::vec3>("uLightDir");
	Uniform<glm::vec3> uLightColorWT = Uniform<glm::vec3>("uLightColor");
	Uniform<GLfloat> uTileSizeWT = Uniform<GLfloat>("uTileSize");
	Uniform<glm::vec3> uViewDirWT = Uniform<glm::vec3>("uViewDir");
	Uniform<glm::vec2> uScreenSizeWT = Uniform<glm::vec2>("uScreenSize");
	Uniform<GLint> uTesselatedTriWidthWT = Uniform<GLint>("uTesselatedTriWidth");
	Uniform<GLfloat> uTexCoordScaleWT = Uniform<GLfloat>("uTexCoordScale");
	Uniform<GLfloat> uDisplacementScaleWT = Uniform<GLfloat>("uDisplacementScale");


	// tildeh0k compute
	Uniform<GLint> uNoiseR0TextureH0C = Uniform<GLint>("uNoiseR0Texture");
	Uniform<GLint> uNoiseI0TextureH0C = Uniform<GLint>("uNoiseI0Texture");
	Uniform<GLint> uNoiseR1TextureH0C = Uniform<GLint>("uNoiseR1Texture");
	Uniform<GLint> uNoiseI1TextureH0C = Uniform<GLint>("uNoiseI1Texture");
	Uniform<GLint> uSimulationResolutionH0C = Uniform<GLint>("uN");
	Uniform<GLint> uWorldSizeH0C = Uniform<GLint>("uL");
	Uniform<GLfloat> uWaveAmplitudeH0C = Uniform<GLfloat>("uA");
	Uniform<glm::vec2> uWindDirectionH0C = Uniform<glm::vec2>("uWindDirection");
	Uniform<GLfloat> uWindSpeedH0C = Uniform<GLfloat>("uWindSpeed");
	Uniform<GLfloat> uWaveSuppressionExpH0C = Uniform<GLfloat>("uWaveSuppressionExp");

	// tildehkt compute
	Uniform<GLint> uSimulationResolutionHTC = Uniform<GLint>("uN");
	Uniform<GLint> uWorldSizeHTC = Uniform<GLint>("uL");
	Uniform<GLfloat> uTimeHTC = Uniform<GLfloat>("uTime");

	// butterflyPrecompute compute
	std::vector<GLint> uJBPC;
	Uniform<GLint> uSimulationResolutionBPC = Uniform<GLint>("uN");

	// butterflyCompute compute
	Uniform<GLint> uStageBCC = Uniform<GLint>("uStage");
	Uniform<GLint> uDirectionBCC = Uniform<GLint>("uDirection");
	Uniform<GLint> uPingPongBCC = Uniform<GLint>("uPingPong");

	// inverse/permute compute
	Uniform<GLint> uSimulationResolutionIPC = Uniform<GLint>("uN");
	Uniform<GLint> uPingPongIPC = Uniform<GLint>("uPingPong");
	Uniform<GLfloat> uChoppinessIPC = Uniform<GLfloat>("uChoppiness");

	// water normal compute
	Uniform<GLfloat> uNormalStrengthNC = Uniform<GLfloat>("uNormalStrength");

	// light volume
	Uniform<GLint> uDisplacementTextureLV = Uniform<GLint>("uDisplacementTexture");
	Uniform<glm::mat4> uInvLightViewProjectionLV = Uniform<glm::mat4>("uInvLightViewProjection");
	Uniform<glm::mat4> uViewProjectionLV = Uniform<glm::mat4>("uViewProjection");
	Uniform<GLint> uPhaseLUTLV = Uniform<GLint>("uPhaseLUT");
	Uniform<glm::vec3> uCamPosLV = Uniform<glm::vec3>("uCamPos");
	Uniform<glm::vec3> uLightIntensitysLV = Uniform<glm::vec3>("uLightIntensity");
	Uniform<glm::vec3> uSigmaExtinctionLV = Uniform<glm::vec3>("uSigmaExtinction");
	Uniform<glm::vec3> uScatterPowerLV = Uniform<glm::vec3>("uScatterPower");
	Uniform<glm::vec3> uLightDirLV = Uniform<glm::vec3>("uLightDir");

	// phase lookup
	Uniform<GLint> uNumPhaseTermsPL = Uniform<GLint>("uNumPhaseTerms");
	std::vector<GLint> uPhaseParamsPL;
	std::vector<GLint> uPhaseFuncPL;

	void createFboAttachments(const std::pair<unsigned int, unsigned int> &_resolution);
	void createWaterAttachments(unsigned int _resolution);
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
	void precomputeFftTextures(const Water &_water);
	void computeFft(const Water &_water);
	void renderWater(const RenderData &_renderData, const std::shared_ptr<Level> &_level);
	void createWaterPlane(const glm::vec2 &_dimensions, GLuint &_VBO, GLuint &_VAO, GLuint &_EBO);
	void createLightVolumeMesh(unsigned int _size, GLuint &_VBO, GLuint &_VAO, GLuint &_EBO);
	void renderLightVolume(const RenderData &_renderData, const std::shared_ptr<Level> &_level);
	void computePhaseLUT();
	void createBrdfLUT();
	bool cullAABB(const glm::mat4 &_mvp, const AxisAlignedBoundingBox &_aabb);
};

