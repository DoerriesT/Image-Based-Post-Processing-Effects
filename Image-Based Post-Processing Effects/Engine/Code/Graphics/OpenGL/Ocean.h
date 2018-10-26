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
struct OceanParams;
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
	bool m_useCompute;
	bool m_useTesselation;
	bool m_wireframe;
	bool m_waterPlaneCreated;
	bool m_tileRingsCreated;

	OceanParams m_currentOceanParams;

	std::shared_ptr<Mesh> m_fullscreenTriangle;

	// compute
	std::shared_ptr<ShaderProgram> m_butterflyPrecomputeCompShader;
	std::shared_ptr<ShaderProgram> m_tildeH0kCompShader;
	std::shared_ptr<ShaderProgram> m_tildeHktCompShader;
	std::shared_ptr<ShaderProgram> m_butterflyComputeCompShader;
	std::shared_ptr<ShaderProgram> m_inversePermuteCompShader;
	std::shared_ptr<ShaderProgram> m_waterNormalCompShader;

	ButterflyComputeRenderPass *m_butterflyComputeRenderPass;
	ButterflyPrecomputeRenderPass *m_butterflyPrecomputeRenderPass;
	InversePermuteRenderPass *m_inversePermuteRenderPass;
	OceanNormalRenderPass *m_oceanNormalRenderPass;
	OceanRenderPass *m_oceanRenderPass;
	OceanTesselationRenderPass *m_oceanTesselationRenderPass;
	TildeH0kRenderPass *m_tildeH0kRenderPass;
	TildeHktRenderPass *m_tildeHktRenderPass;

	TileRing *m_tileRings[6];

	GLuint m_oceanVBO;
	GLuint m_oceanVAO;
	GLuint m_oceanEBO;

	// fft fbo
	GLuint m_fftFbo;
	GLuint m_tildeH0kTexture;
	GLuint m_tildeH0minusKTexture;
	GLuint m_tildeHktDxTexture;
	GLuint m_tildeHktDyTexture;
	GLuint m_tildeHktDzTexture;
	GLuint m_pingPongTextureA;
	GLuint m_pingPongTextureB;
	GLuint m_pingPongTextureC;

	// fft-twiddle indices fbo
	GLuint m_twiddleIndicesFbo;
	GLuint m_twiddleIndicesTexture;

	// displacement fbo
	GLuint m_oceanFbo;
	GLuint m_oceanDisplacementFoldingTexture;
	GLuint m_oceanNormalTexture;

	// tildeh0k compute
	Uniform<GLint> m_uSimulationResolutionH0C = Uniform<GLint>("uN");
	Uniform<GLint> m_uWorldSizeH0C = Uniform<GLint>("uL");
	Uniform<GLfloat> m_uWaveAmplitudeH0C = Uniform<GLfloat>("uA");
	Uniform<glm::vec2> m_uWindDirectionH0C = Uniform<glm::vec2>("uWindDirection");
	Uniform<GLfloat> m_uWindSpeedH0C = Uniform<GLfloat>("uWindSpeed");
	Uniform<GLfloat> m_uWaveSuppressionExpH0C = Uniform<GLfloat>("uWaveSuppressionExp");

	// tildehkt compute
	Uniform<GLint> m_uSimulationResolutionHTC = Uniform<GLint>("uN");
	Uniform<GLint> m_uWorldSizeHTC = Uniform<GLint>("uL");
	Uniform<GLfloat> m_uTimeHTC = Uniform<GLfloat>("uTime");

	// butterflyPrecompute compute
	std::vector<GLint> m_uJBPC;
	Uniform<GLint> m_uSimulationResolutionBPC = Uniform<GLint>("uN");

	// butterflyCompute compute
	Uniform<GLint> m_uStageBCC = Uniform<GLint>("uStage");
	Uniform<GLint> m_uDirectionBCC = Uniform<GLint>("uDirection");
	Uniform<GLint> m_uPingPongBCC = Uniform<GLint>("uPingPong");

	// inverse/permute compute
	Uniform<GLint> m_uSimulationResolutionIPC = Uniform<GLint>("uN");
	Uniform<GLint> m_uPingPongIPC = Uniform<GLint>("uPingPong");
	Uniform<GLfloat> m_uChoppinessIPC = Uniform<GLfloat>("uChoppiness");

	// water normal compute
	Uniform<GLfloat> m_uNormalStrengthNC = Uniform<GLfloat>("uNormalStrength");

	void createWaterAttachments(unsigned int _resolution);
	void precomputeFftTextures(const OceanParams &_water, RenderPass **_previousRenderPass = nullptr);
	void computeFft(const OceanParams &_water, RenderPass **_previousRenderPass = nullptr);
	void createWaterPlane();
	void createTileRings();
};