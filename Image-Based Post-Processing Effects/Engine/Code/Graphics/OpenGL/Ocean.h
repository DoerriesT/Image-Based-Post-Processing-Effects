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
class RenderPass;
class ButterflyComputeRenderPass;
class ButterflyPrecomputeRenderPass;
class InversePermuteRenderPass;
class OceanNormalRenderPass;
class OceanRenderPass;
class OceanTesselationRenderPass;
class TildeH0kRenderPass;
class TildeHktRenderPass;

class Ocean
{
public:
	explicit Ocean(bool _useCompute, bool _useTesselation, bool _wireframe = false);
	Ocean(const Ocean &) = delete;
	Ocean(const Ocean &&) = delete;
	Ocean &operator= (const Ocean &) = delete;
	Ocean &operator= (const Ocean &&) = delete;
	~Ocean();
	void init(GLuint _gbufferFbo, unsigned int _width, unsigned int _height);
	void prepareRender(const RenderData &_renderData, const std::shared_ptr<Level> &_level, RenderPass **_previousRenderPass = nullptr);
	void render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, RenderPass **_previousRenderPass = nullptr);
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

	// compute
	std::shared_ptr<ShaderProgram> butterflyPrecomputeCompShader;
	std::shared_ptr<ShaderProgram> tildeH0kCompShader;
	std::shared_ptr<ShaderProgram> tildeHktCompShader;
	std::shared_ptr<ShaderProgram> butterflyComputeCompShader;
	std::shared_ptr<ShaderProgram> inversePermuteCompShader;
	std::shared_ptr<ShaderProgram> waterNormalCompShader;

	ButterflyComputeRenderPass *butterflyComputeRenderPass;
	ButterflyPrecomputeRenderPass *butterflyPrecomputeRenderPass;
	InversePermuteRenderPass *inversePermuteRenderPass;
	OceanNormalRenderPass *oceanNormalRenderPass;
	OceanRenderPass *oceanRenderPass;
	OceanTesselationRenderPass *oceanTesselationRenderPass;
	TildeH0kRenderPass *tildeH0kRenderPass;
	TildeHktRenderPass *tildeHktRenderPass;

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

	void createWaterAttachments(unsigned int _resolution);
	void precomputeFftTextures(const Water &_water, RenderPass **_previousRenderPass = nullptr);
	void computeFft(const Water &_water, RenderPass **_previousRenderPass = nullptr);
	void createWaterPlane();
	void createTileRings();
};