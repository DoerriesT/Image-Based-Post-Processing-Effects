#pragma once
#include <memory>
#include <glad\glad.h>
#include "Level.h"
#include "Uniform.h"

class ShaderProgram;
class TileRing;
class Scene;
struct RenderData;
struct Effects;
struct Water;

class Ocean
{
public:
	explicit Ocean(bool _useCompute, bool _useTesselation, bool _wireframe = false);
	Ocean(const Ocean &) = delete;
	Ocean(const Ocean &&) = delete;
	Ocean &operator= (const Ocean &) = delete;
	Ocean &operator= (const Ocean &&) = delete;
	~Ocean();
	void init();
	void prepareRender(const RenderData &_renderData, const std::shared_ptr<Level> &_level);
	void render(const RenderData &_renderData, const std::shared_ptr<Level> &_level);
	void setCompute(bool _useCompute);
	void setTesselation(bool _usetesselation);
	void setWireframe(bool _wireframe);
	bool isCompute() const;
	bool isTesselation() const;
	bool isWireframe() const;

private:
	bool useCompute;
	bool useTesselation;
	bool wireframe;
	bool waterPlaneCreated;
	bool tileRingsCreated;

	Water currentWaterConfig;

	std::shared_ptr<Mesh> fullscreenTriangle;

	// fragment
	std::shared_ptr<ShaderProgram> butterflyPrecomputeShader;
	std::shared_ptr<ShaderProgram> tildeH0kShader;
	std::shared_ptr<ShaderProgram> tildeHktShader;
	std::shared_ptr<ShaderProgram> butterflyComputeShader;
	std::shared_ptr<ShaderProgram> inversePermuteShader;
	std::shared_ptr<ShaderProgram> waterNormalShader;

	// compute
	std::shared_ptr<ShaderProgram> butterflyPrecomputeCompShader;
	std::shared_ptr<ShaderProgram> tildeH0kCompShader;
	std::shared_ptr<ShaderProgram> tildeHktCompShader;
	std::shared_ptr<ShaderProgram> butterflyComputeCompShader;
	std::shared_ptr<ShaderProgram> inversePermuteCompShader;
	std::shared_ptr<ShaderProgram> waterNormalCompShader;

	// tesselation
	std::shared_ptr<ShaderProgram> waterTessShader;

	// world space grid
	std::shared_ptr<ShaderProgram> waterShader;

	TileRing *tileRings[6];

	GLuint waterVBO;
	GLuint waterVAO;
	GLuint waterEBO;

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

	// tildeh0k
	Uniform<GLint> uSimulationResolutionH0 = Uniform<GLint>("uN");
	Uniform<GLint> uWorldSizeH0 = Uniform<GLint>("uL");
	Uniform<GLfloat> uWaveAmplitudeH0 = Uniform<GLfloat>("uA");
	Uniform<glm::vec2> uWindDirectionH0 = Uniform<glm::vec2>("uWindDirection");
	Uniform<GLfloat> uWindSpeedH0 = Uniform<GLfloat>("uWindSpeed");
	Uniform<GLfloat> uWaveSuppressionExpH0 = Uniform<GLfloat>("uWaveSuppressionExp");

	// tildehkt
	Uniform<GLint> uSimulationResolutionHT = Uniform<GLint>("uN");
	Uniform<GLint> uWorldSizeHT = Uniform<GLint>("uL");
	Uniform<GLfloat> uTimeHT = Uniform<GLfloat>("uTime");

	// butterflyPrecompute
	std::vector<GLint> uJBP;
	Uniform<GLint> uSimulationResolutionBP = Uniform<GLint>("uN");

	// butterflyCompute
	Uniform<GLint> uSimulationResolutionBC = Uniform<GLint>("uN");
	Uniform<GLint> uStageBC = Uniform<GLint>("uStage");
	Uniform<GLint> uStagesBC = Uniform<GLint>("uStages");
	Uniform<GLint> uDirectionBC = Uniform<GLint>("uDirection");

	// inverse/permute
	Uniform<GLint> uSimulationResolutionIP = Uniform<GLint>("uN");
	Uniform<GLfloat> uChoppinessIP = Uniform<GLfloat>("uChoppiness");

	// water normal
	Uniform<GLfloat> uNormalStrengthN = Uniform<GLfloat>("uNormalStrength");

	// tildeh0k compute
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

	// water
	Uniform<glm::mat4> uProjectionW = Uniform<glm::mat4>("uProjection");
	Uniform<glm::mat4> uViewW = Uniform<glm::mat4>("uView");
	Uniform<glm::vec3> uCamPosW = Uniform<glm::vec3>("uCamPos");
	Uniform<glm::vec2> uTexCoordShiftW = Uniform<glm::vec2>("uTexCoordShift");
	Uniform<bool> uUseEnvironmentW = Uniform<bool>("uUseEnvironment");
	Uniform<float> uWaterLevelW = Uniform<float>("uWaterLevel");
	Uniform<glm::vec3> uLightDirW = Uniform<glm::vec3>("uLightDir");
	Uniform<glm::vec3> uLightColorW = Uniform<glm::vec3>("uLightColor");

	// water tesselated
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

	void createWaterAttachments(unsigned int _resolution);
	void precomputeFftTextures(const Water &_water);
	void computeFft(const Water &_water);
	void createWaterPlane();
	void createTileRings();
};