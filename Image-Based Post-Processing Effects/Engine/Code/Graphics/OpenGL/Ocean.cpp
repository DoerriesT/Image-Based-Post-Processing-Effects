#include "Ocean.h"
#include"Graphics\Texture.h"
#include <glm\ext.hpp>
#include "Engine.h"
#include "TileRing.h"
#include "Graphics\Mesh.h"
#include "RenderData.h"
#include "Graphics\OpenGL\RenderPass\Ocean\ButterflyComputeRenderPass.h"
#include "Graphics\OpenGL\RenderPass\Ocean\ButterflyPrecomputeRenderPass.h"
#include "Graphics\OpenGL\RenderPass\Ocean\InversePermuteRenderPass.h"
#include "Graphics\OpenGL\RenderPass\Ocean\OceanNormalRenderPass.h"
#include "Graphics\OpenGL\RenderPass\Ocean\OceanRenderPass.h"
#include "Graphics\OpenGL\RenderPass\Ocean\OceanTesselationRenderPass.h"
#include "Graphics\OpenGL\RenderPass\Ocean\TildeH0kRenderPass.h"
#include "Graphics\OpenGL\RenderPass\Ocean\TildeHktRenderPass.h"

Ocean::Ocean(bool _useCompute, bool _useTesselation, bool _wireframe)
	:m_useCompute(_useCompute), m_useTesselation(_useTesselation), m_wireframe(_wireframe)
{
}

Ocean::~Ocean()
{
	if (m_waterPlaneCreated)
	{
		// delete water mesh
		glBindVertexArray(m_oceanVAO);
		glDisableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &m_oceanEBO);
		glDeleteBuffers(1, &m_oceanVBO);
		glBindVertexArray(0);
		glDeleteVertexArrays(1, &m_oceanVAO);
	}
	if (m_tileRingsCreated)
	{
		for (int i = 0; i < 6; ++i)
		{
			delete m_tileRings[i];
		}
	}
}

void Ocean::init(GLuint _gbufferFbo, unsigned int _width, unsigned int _height)
{

	// compute
	m_tildeH0kCompShader = ShaderProgram::createShaderProgram("Resources/Shaders/Ocean/tildeH0k.comp");
	m_tildeHktCompShader = ShaderProgram::createShaderProgram("Resources/Shaders/Ocean/tildeHkt.comp");
	m_butterflyPrecomputeCompShader = ShaderProgram::createShaderProgram("Resources/Shaders/Ocean/butterflyPrecompute.comp");
	m_butterflyComputeCompShader = ShaderProgram::createShaderProgram("Resources/Shaders/Ocean/butterflyCompute.comp");
	m_inversePermuteCompShader = ShaderProgram::createShaderProgram("Resources/Shaders/Ocean/inversePermute.comp");
	m_waterNormalCompShader = ShaderProgram::createShaderProgram("Resources/Shaders/Ocean/normal.comp");

	// tildeh0k compute
	m_uSimulationResolutionH0C.create(m_tildeH0kCompShader);
	m_uWorldSizeH0C.create(m_tildeH0kCompShader);
	m_uWaveAmplitudeH0C.create(m_tildeH0kCompShader);
	m_uWindDirectionH0C.create(m_tildeH0kCompShader);
	m_uWindSpeedH0C.create(m_tildeH0kCompShader);
	m_uWaveSuppressionExpH0C.create(m_tildeH0kCompShader);

	// tildehkt compute
	m_uSimulationResolutionHTC.create(m_tildeHktCompShader);
	m_uWorldSizeHTC.create(m_tildeHktCompShader);
	m_uTimeHTC.create(m_tildeHktCompShader);

	// butterflyPrecompute compute
	for (int i = 0; i < 512; ++i)
	{
		m_uJBPC.push_back(m_butterflyPrecomputeCompShader->createUniform(std::string("uJ") + "[" + std::to_string(i) + "]"));
	}
	m_uSimulationResolutionBPC.create(m_butterflyPrecomputeCompShader);

	// butterflyCompute compute
	m_uStageBCC.create(m_butterflyComputeCompShader);
	m_uDirectionBCC.create(m_butterflyComputeCompShader);
	m_uPingPongBCC.create(m_butterflyComputeCompShader);

	// inverse/permute compute
	m_uSimulationResolutionIPC.create(m_inversePermuteCompShader);
	m_uPingPongIPC.create(m_inversePermuteCompShader);
	m_uChoppinessIPC.create(m_inversePermuteCompShader);

	// water normal compute
	m_uNormalStrengthNC.create(m_waterNormalCompShader);

	// create FBO
	glGenFramebuffers(1, &m_fftFbo);
	glGenFramebuffers(1, &m_twiddleIndicesFbo);
	glGenFramebuffers(1, &m_oceanFbo);

	m_butterflyComputeRenderPass = new ButterflyComputeRenderPass(m_fftFbo, m_currentOceanParams.m_simulationResolution, m_currentOceanParams.m_simulationResolution);
	m_butterflyPrecomputeRenderPass = new ButterflyPrecomputeRenderPass(m_twiddleIndicesFbo, glm::log2(m_currentOceanParams.m_simulationResolution), m_currentOceanParams.m_simulationResolution);
	m_inversePermuteRenderPass = new InversePermuteRenderPass(m_oceanFbo, m_currentOceanParams.m_simulationResolution, m_currentOceanParams.m_simulationResolution);
	m_oceanNormalRenderPass = new OceanNormalRenderPass(m_oceanFbo, m_currentOceanParams.m_simulationResolution, m_currentOceanParams.m_simulationResolution);
	m_oceanRenderPass = new OceanRenderPass(_gbufferFbo, _width, _height);
	m_oceanTesselationRenderPass = new OceanTesselationRenderPass(_gbufferFbo, _width, _height);
	m_tildeH0kRenderPass = new TildeH0kRenderPass(m_fftFbo, m_currentOceanParams.m_simulationResolution, m_currentOceanParams.m_simulationResolution);
	m_tildeHktRenderPass = new TildeHktRenderPass(m_fftFbo, m_currentOceanParams.m_simulationResolution, m_currentOceanParams.m_simulationResolution);

	m_fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void Ocean::prepareRender(const RenderData &_renderData, const std::shared_ptr<Level> &_level, RenderPass **_previousRenderPass)
{
	bool recompute = false;

	OceanParams &water = _level->m_oceanParams;

	if (m_currentOceanParams.m_simulationResolution != water.m_simulationResolution)
	{
		m_currentOceanParams.m_simulationResolution = water.m_simulationResolution;
		recompute = true;
		createWaterAttachments(m_currentOceanParams.m_simulationResolution);

		m_butterflyComputeRenderPass->resize(m_currentOceanParams.m_simulationResolution, m_currentOceanParams.m_simulationResolution);
		m_butterflyPrecomputeRenderPass->resize(glm::log2(m_currentOceanParams.m_simulationResolution), m_currentOceanParams.m_simulationResolution);
		m_inversePermuteRenderPass->resize(m_currentOceanParams.m_simulationResolution, m_currentOceanParams.m_simulationResolution);
		m_oceanNormalRenderPass->resize(m_currentOceanParams.m_simulationResolution, m_currentOceanParams.m_simulationResolution);
		m_tildeH0kRenderPass->resize(m_currentOceanParams.m_simulationResolution, m_currentOceanParams.m_simulationResolution);
		m_tildeHktRenderPass->resize(m_currentOceanParams.m_simulationResolution, m_currentOceanParams.m_simulationResolution);
	}

	if (m_currentOceanParams.m_normalizedWindDirection != water.m_normalizedWindDirection ||
		m_currentOceanParams.m_waveAmplitude != water.m_waveAmplitude ||
		m_currentOceanParams.m_waveSuppressionExponent != water.m_waveSuppressionExponent ||
		m_currentOceanParams.m_windSpeed != water.m_windSpeed ||
		m_currentOceanParams.m_worldSize != water.m_worldSize)
	{
		recompute = true;
	}

	m_currentOceanParams = water;

	if (recompute)
	{
		precomputeFftTextures(m_currentOceanParams, _previousRenderPass);
	}

	computeFft(m_currentOceanParams, _previousRenderPass);
}

void Ocean::render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, RenderPass **_previousRenderPass)
{
	static std::shared_ptr<Texture> foamTexture = Texture::createTexture("Resources/Textures/foam.dds", true);

	if (m_useTesselation)
	{
		if (!m_tileRingsCreated)
		{
			createTileRings();
		}

		m_oceanTesselationRenderPass->render(_renderData, _level, m_oceanDisplacementFoldingTexture, m_oceanNormalTexture, m_tileRings, m_wireframe, _previousRenderPass);
	}
	else
	{
		if (!m_waterPlaneCreated)
		{
			createWaterPlane();
		}

		m_oceanRenderPass->render(_renderData, _level, m_oceanDisplacementFoldingTexture, m_oceanNormalTexture, m_oceanVAO, 300, m_wireframe, _previousRenderPass);
	}
}

void Ocean::setCompute(bool _useCompute)
{
	m_useCompute = _useCompute;
}

void Ocean::setTesselation(bool _usetesselation)
{
	m_useTesselation = _usetesselation;
}

void Ocean::setWireframe(bool _wireframe)
{
	m_wireframe = _wireframe;
}

bool Ocean::isCompute() const
{
	return m_useCompute;
}

bool Ocean::isTesselation() const
{
	return m_useTesselation;
}

bool Ocean::isWireframe() const
{
	return m_wireframe;
}

void Ocean::createWaterAttachments(unsigned int _resolution)
{
	GLuint textures[] = { m_tildeH0kTexture, m_tildeH0minusKTexture, m_tildeHktDxTexture, m_tildeHktDyTexture, m_tildeHktDzTexture, m_pingPongTextureA, m_pingPongTextureB, m_pingPongTextureC,
		m_twiddleIndicesTexture, m_oceanDisplacementFoldingTexture, m_oceanNormalTexture };
	glDeleteTextures(sizeof(textures) / sizeof(GLuint), textures);

	glBindFramebuffer(GL_FRAMEBUFFER, m_fftFbo);

	glGenTextures(1, &m_tildeH0kTexture);
	glBindTexture(GL_TEXTURE_2D, m_tildeH0kTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, _resolution, _resolution, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tildeH0kTexture, 0);

	glGenTextures(1, &m_tildeH0minusKTexture);
	glBindTexture(GL_TEXTURE_2D, m_tildeH0minusKTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, _resolution, _resolution, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_tildeH0minusKTexture, 0);

	glGenTextures(1, &m_tildeHktDxTexture);
	glBindTexture(GL_TEXTURE_2D, m_tildeHktDxTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, _resolution, _resolution, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_tildeHktDxTexture, 0);

	glGenTextures(1, &m_tildeHktDyTexture);
	glBindTexture(GL_TEXTURE_2D, m_tildeHktDyTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, _resolution, _resolution, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_tildeHktDyTexture, 0);

	glGenTextures(1, &m_tildeHktDzTexture);
	glBindTexture(GL_TEXTURE_2D, m_tildeHktDzTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, _resolution, _resolution, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, m_tildeHktDzTexture, 0);

	glGenTextures(1, &m_pingPongTextureA);
	glBindTexture(GL_TEXTURE_2D, m_pingPongTextureA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, _resolution, _resolution, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, m_pingPongTextureA, 0);

	glGenTextures(1, &m_pingPongTextureB);
	glBindTexture(GL_TEXTURE_2D, m_pingPongTextureB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, _resolution, _resolution, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, m_pingPongTextureB, 0);

	glGenTextures(1, &m_pingPongTextureC);
	glBindTexture(GL_TEXTURE_2D, m_pingPongTextureC);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, _resolution, _resolution, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT7, GL_TEXTURE_2D, m_pingPongTextureC, 0);


	glBindFramebuffer(GL_FRAMEBUFFER, m_twiddleIndicesFbo);

	glGenTextures(1, &m_twiddleIndicesTexture);
	glBindTexture(GL_TEXTURE_2D, m_twiddleIndicesTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, glm::log2(_resolution), _resolution, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_twiddleIndicesTexture, 0);


	glBindFramebuffer(GL_FRAMEBUFFER, m_oceanFbo);

	glGenTextures(1, &m_oceanDisplacementFoldingTexture);
	glBindTexture(GL_TEXTURE_2D, m_oceanDisplacementFoldingTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _resolution, _resolution, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_oceanDisplacementFoldingTexture, 0);

	glGenTextures(1, &m_oceanNormalTexture);
	glBindTexture(GL_TEXTURE_2D, m_oceanNormalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _resolution, _resolution, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_oceanNormalTexture, 0);
}

void Ocean::precomputeFftTextures(const OceanParams & _water, RenderPass **_previousRenderPass)
{
	if (m_useCompute)
	{
		// tildeh0k/minusk
		{
			std::shared_ptr<Texture> noise0;
			std::shared_ptr<Texture> noise1;
			std::shared_ptr<Texture> noise2;
			std::shared_ptr<Texture> noise3;

			// consider keeping these textures permanently in memory
			switch (_water.m_simulationResolution)
			{
			case 512:
				noise0 = Texture::createTexture("Resources/Textures/Noise512_0.dds", true);
				noise1 = Texture::createTexture("Resources/Textures/Noise512_1.dds", true);
				noise2 = Texture::createTexture("Resources/Textures/Noise512_2.dds", true);
				noise3 = Texture::createTexture("Resources/Textures/Noise512_3.dds", true);
				break;
			case 256:
			default:
				noise0 = Texture::createTexture("Resources/Textures/Noise256_0.dds", true);
				noise1 = Texture::createTexture("Resources/Textures/Noise256_1.dds", true);
				noise2 = Texture::createTexture("Resources/Textures/Noise256_2.dds", true);
				noise3 = Texture::createTexture("Resources/Textures/Noise256_3.dds", true);
				break;
			}

			m_tildeH0kCompShader->bind();

			m_uSimulationResolutionH0C.set(_water.m_simulationResolution);
			m_uWorldSizeH0C.set(_water.m_worldSize);
			m_uWaveAmplitudeH0C.set(_water.m_waveAmplitude);
			m_uWindDirectionH0C.set(_water.m_normalizedWindDirection);
			m_uWindSpeedH0C.set(_water.m_windSpeed);
			m_uWaveSuppressionExpH0C.set(_water.m_waveSuppressionExponent);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, noise0->getId());
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, noise1->getId());
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, noise2->getId());
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, noise3->getId());

			glBindImageTexture(0, m_tildeH0kTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
			glBindImageTexture(1, m_tildeH0minusKTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
			glDispatchCompute(_water.m_simulationResolution / 8, _water.m_simulationResolution / 8, 1);
			glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
		}

		// butterfly precompute
		{
			std::unique_ptr<uint32_t[]> bitReversedIndices = std::make_unique<uint32_t[]>(static_cast<size_t>(_water.m_simulationResolution));

			for (std::uint32_t i = 0; i < _water.m_simulationResolution; ++i)
			{
				std::uint32_t x = glm::bitfieldReverse(i);
				x = glm::bitfieldRotateRight(x, glm::log2(_water.m_simulationResolution));
				bitReversedIndices[static_cast<size_t>(i)] = x;
			}

			m_butterflyPrecomputeCompShader->bind();
			m_uSimulationResolutionBPC.set(_water.m_simulationResolution);
			for (size_t i = 0; i < static_cast<size_t>(_water.m_simulationResolution); ++i)
			{
				m_butterflyPrecomputeCompShader->setUniform(m_uJBPC[i], (int)bitReversedIndices[i]);
			}

			glBindImageTexture(0, m_twiddleIndicesTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
			glDispatchCompute(glm::log2(_water.m_simulationResolution), _water.m_simulationResolution / 8, 1);
			glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
		}
	}
	else
	{
		m_tildeH0kRenderPass->render(_water, _previousRenderPass);
		m_butterflyPrecomputeRenderPass->render(_water, _previousRenderPass);
	}
}

void Ocean::computeFft(const OceanParams & _water, RenderPass **_previousRenderPass)
{
	if (m_useCompute)
	{
		// tildehkt
		{
			m_tildeHktCompShader->bind();
			m_uSimulationResolutionHTC.set(_water.m_simulationResolution);
			m_uWorldSizeHTC.set(_water.m_worldSize);
			m_uTimeHTC.set((float)Engine::getTime() * _water.m_timeScale);

			glBindImageTexture(0, m_tildeHktDxTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG16F);
			glBindImageTexture(1, m_tildeHktDyTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG16F);
			glBindImageTexture(2, m_tildeHktDzTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG16F);
			glBindImageTexture(3, m_tildeH0kTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG16F);
			glBindImageTexture(4, m_tildeH0minusKTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG16F);
			glDispatchCompute(_water.m_simulationResolution / 8, _water.m_simulationResolution / 8, 1);
			glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
		}

		// butterfly computation/ inversion
		{
			m_butterflyComputeCompShader->bind();


			glBindImageTexture(3, m_pingPongTextureA, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG16F);
			glBindImageTexture(4, m_pingPongTextureB, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG16F);
			glBindImageTexture(5, m_pingPongTextureC, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG16F);
			glBindImageTexture(6, m_twiddleIndicesTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);

			int pingpong = 0;

			for (int i = 0; i < 2; ++i)
			{
				m_uDirectionBCC.set(i);

				for (int j = 0; unsigned int(j) < glm::log2(_water.m_simulationResolution); ++j)
				{
					m_uStageBCC.set(j);
					m_uPingPongBCC.set(pingpong);

					glDispatchCompute(_water.m_simulationResolution / 8, _water.m_simulationResolution / 8, 1);
					glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

					pingpong = 1 - pingpong;
				}
			}

			// inverse/permute
			{
				m_inversePermuteCompShader->bind();
				m_uSimulationResolutionIPC.set(_water.m_simulationResolution);
				m_uPingPongIPC.set(pingpong);
				m_uChoppinessIPC.set(-_water.m_waveChoppiness);

				glBindImageTexture(6, m_oceanDisplacementFoldingTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);

				glDispatchCompute(_water.m_simulationResolution / 8, _water.m_simulationResolution / 8, 1);
				glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

				// generate mips
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, m_oceanDisplacementFoldingTexture);
				glGenerateMipmap(GL_TEXTURE_2D);
			}

			// normal
			{
				m_waterNormalCompShader->bind();

				m_uNormalStrengthNC.set(_water.m_normalStrength);

				glBindImageTexture(0, m_oceanNormalTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);

				glDispatchCompute(_water.m_simulationResolution / 8, _water.m_simulationResolution / 8, 1);
				glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

				// generate mips
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, m_oceanNormalTexture);
				glGenerateMipmap(GL_TEXTURE_2D);
			}
		}
	}
	else
	{
		GLuint readTextures[] = { m_tildeHktDxTexture, m_tildeHktDyTexture, m_tildeHktDzTexture , m_pingPongTextureA, m_pingPongTextureB, m_pingPongTextureC };

		m_tildeHktRenderPass->render(_water, m_tildeH0kTexture, m_tildeH0minusKTexture, _previousRenderPass);
		m_butterflyComputeRenderPass->render(_water, m_twiddleIndicesTexture, readTextures, _previousRenderPass);
		m_inversePermuteRenderPass->render(_water, readTextures, m_oceanDisplacementFoldingTexture, _previousRenderPass);
		m_oceanNormalRenderPass->render(_water, m_oceanDisplacementFoldingTexture, m_oceanNormalTexture, _previousRenderPass);
	}
}

void Ocean::createWaterPlane()
{
	m_waterPlaneCreated = true;

	const unsigned int WATER_PLANE_DIMENSION = 300;

	std::vector<glm::vec2> vertices(std::size_t((WATER_PLANE_DIMENSION + 1) * ((WATER_PLANE_DIMENSION + 1))));
	for (std::size_t i = 0, y = 0; y <= WATER_PLANE_DIMENSION; ++y)
	{
		for (std::size_t x = 0; x <= WATER_PLANE_DIMENSION; ++x, ++i)
		{
			glm::vec2 pos(x, y);
			pos /= WATER_PLANE_DIMENSION;
			pos = pos * 2.0 - glm::vec2(1.0);
			vertices[i] = pos;
		}
	}

	std::vector<GLuint> indices(std::size_t(WATER_PLANE_DIMENSION * WATER_PLANE_DIMENSION * 6));
	for (std::size_t ti = 0, vi = 0, y = 0; y < WATER_PLANE_DIMENSION; ++y, ++vi)
	{
		for (int x = 0; x < WATER_PLANE_DIMENSION; ++x, ti += 6, ++vi)
		{
			indices[ti] = GLuint(vi);
			indices[ti + 3] = indices[ti + 2] = GLuint(vi + 1);
			indices[ti + 4] = indices[ti + 1] = GLuint(vi + WATER_PLANE_DIMENSION + 1);
			indices[ti + 5] = GLuint(vi + WATER_PLANE_DIMENSION + 2);
		}
	}

	glGenVertexArrays(1, &m_oceanVAO);
	glGenBuffers(1, &m_oceanVBO);
	glGenBuffers(1, &m_oceanEBO);

	glBindVertexArray(m_oceanVAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_oceanVBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_oceanEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
}

void Ocean::createTileRings()
{
	m_tileRingsCreated = true;

	// init terrain tile rings
	float tileWidth = 16.0f;
	int ringWidth = 16;
	for (int i = 0; i < 6; ++i)
	{
		int innerWidth = (i == 0) ? 0 : ringWidth / 2;
		int outerWidth = ringWidth;
		m_tileRings[i] = new TileRing(innerWidth, outerWidth, tileWidth);
		tileWidth *= 2.0f;
	}
}
