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
	:useCompute(_useCompute), useTesselation(_useTesselation), wireframe(_wireframe)
{
}

Ocean::~Ocean()
{
	if (waterPlaneCreated)
	{
		// delete water mesh
		glBindVertexArray(waterVAO);
		glDisableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &waterEBO);
		glDeleteBuffers(1, &waterVBO);
		glBindVertexArray(0);
		glDeleteVertexArrays(1, &waterVAO);
	}
	if (tileRingsCreated)
	{
		for (int i = 0; i < 6; ++i)
		{
			delete tileRings[i];
		}
	}
}

void Ocean::init(GLuint _gbufferFbo, unsigned int _width, unsigned int _height)
{

	// compute
	tildeH0kCompShader = ShaderProgram::createShaderProgram("Resources/Shaders/Ocean/tildeH0k.comp");
	tildeHktCompShader = ShaderProgram::createShaderProgram("Resources/Shaders/Ocean/tildeHkt.comp");
	butterflyPrecomputeCompShader = ShaderProgram::createShaderProgram("Resources/Shaders/Ocean/butterflyPrecompute.comp");
	butterflyComputeCompShader = ShaderProgram::createShaderProgram("Resources/Shaders/Ocean/butterflyCompute.comp");
	inversePermuteCompShader = ShaderProgram::createShaderProgram("Resources/Shaders/Ocean/inversePermute.comp");
	waterNormalCompShader = ShaderProgram::createShaderProgram("Resources/Shaders/Ocean/normal.comp");

	// tildeh0k compute
	uSimulationResolutionH0C.create(tildeH0kCompShader);
	uWorldSizeH0C.create(tildeH0kCompShader);
	uWaveAmplitudeH0C.create(tildeH0kCompShader);
	uWindDirectionH0C.create(tildeH0kCompShader);
	uWindSpeedH0C.create(tildeH0kCompShader);
	uWaveSuppressionExpH0C.create(tildeH0kCompShader);

	// tildehkt compute
	uSimulationResolutionHTC.create(tildeHktCompShader);
	uWorldSizeHTC.create(tildeHktCompShader);
	uTimeHTC.create(tildeHktCompShader);

	// butterflyPrecompute compute
	for (int i = 0; i < 512; ++i)
	{
		uJBPC.push_back(butterflyPrecomputeCompShader->createUniform(std::string("uJ") + "[" + std::to_string(i) + "]"));
	}
	uSimulationResolutionBPC.create(butterflyPrecomputeCompShader);

	// butterflyCompute compute
	uStageBCC.create(butterflyComputeCompShader);
	uDirectionBCC.create(butterflyComputeCompShader);
	uPingPongBCC.create(butterflyComputeCompShader);

	// inverse/permute compute
	uSimulationResolutionIPC.create(inversePermuteCompShader);
	uPingPongIPC.create(inversePermuteCompShader);
	uChoppinessIPC.create(inversePermuteCompShader);

	// water normal compute
	uNormalStrengthNC.create(waterNormalCompShader);

	// create FBO
	glGenFramebuffers(1, &fftFbo);
	glGenFramebuffers(1, &twiddleIndicesFbo);
	glGenFramebuffers(1, &waterFbo);

	butterflyComputeRenderPass = new ButterflyComputeRenderPass(fftFbo, currentWaterConfig.simulationResolution, currentWaterConfig.simulationResolution);
	butterflyPrecomputeRenderPass = new ButterflyPrecomputeRenderPass(twiddleIndicesFbo, glm::log2(currentWaterConfig.simulationResolution), currentWaterConfig.simulationResolution);
	inversePermuteRenderPass = new InversePermuteRenderPass(waterFbo, currentWaterConfig.simulationResolution, currentWaterConfig.simulationResolution);
	oceanNormalRenderPass = new OceanNormalRenderPass(waterFbo, currentWaterConfig.simulationResolution, currentWaterConfig.simulationResolution);
	oceanRenderPass = new OceanRenderPass(_gbufferFbo, _width, _height);
	oceanTesselationRenderPass = new OceanTesselationRenderPass(_gbufferFbo, _width, _height);
	tildeH0kRenderPass = new TildeH0kRenderPass(fftFbo, currentWaterConfig.simulationResolution, currentWaterConfig.simulationResolution);
	tildeHktRenderPass = new TildeHktRenderPass(fftFbo, currentWaterConfig.simulationResolution, currentWaterConfig.simulationResolution);

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void Ocean::prepareRender(const RenderData &_renderData, const std::shared_ptr<Level> &_level, RenderPass **_previousRenderPass)
{
	bool recompute = false;

	Water &water = _level->water;

	if (currentWaterConfig.simulationResolution != water.simulationResolution)
	{
		currentWaterConfig.simulationResolution = water.simulationResolution;
		recompute = true;
		createWaterAttachments(currentWaterConfig.simulationResolution);

		butterflyComputeRenderPass->resize(currentWaterConfig.simulationResolution, currentWaterConfig.simulationResolution);
		butterflyPrecomputeRenderPass->resize(glm::log2(currentWaterConfig.simulationResolution), currentWaterConfig.simulationResolution);
		inversePermuteRenderPass->resize(currentWaterConfig.simulationResolution, currentWaterConfig.simulationResolution);
		oceanNormalRenderPass->resize(currentWaterConfig.simulationResolution, currentWaterConfig.simulationResolution);
		tildeH0kRenderPass->resize(currentWaterConfig.simulationResolution, currentWaterConfig.simulationResolution);
		tildeHktRenderPass->resize(currentWaterConfig.simulationResolution, currentWaterConfig.simulationResolution);
	}

	if (currentWaterConfig.normalizedWindDirection != water.normalizedWindDirection ||
		currentWaterConfig.waveAmplitude != water.waveAmplitude ||
		currentWaterConfig.waveSuppressionExponent != water.waveSuppressionExponent ||
		currentWaterConfig.windSpeed != water.windSpeed ||
		currentWaterConfig.worldSize != water.worldSize)
	{
		recompute = true;
	}

	currentWaterConfig = water;

	if (recompute)
	{
		precomputeFftTextures(currentWaterConfig, _previousRenderPass);
	}

	computeFft(currentWaterConfig, _previousRenderPass);
}

void Ocean::render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, RenderPass **_previousRenderPass)
{
	static std::shared_ptr<Texture> foamTexture = Texture::createTexture("Resources/Textures/foam.dds", true);

	if (useTesselation)
	{
		if (!tileRingsCreated)
		{
			createTileRings();
		}

		oceanTesselationRenderPass->render(_renderData, _level, waterDisplacementFoldingTexture, waterNormalTexture, tileRings, wireframe, _previousRenderPass);
	}
	else
	{
		if (!waterPlaneCreated)
		{
			createWaterPlane();
		}

		oceanRenderPass->render(_renderData, _level, waterDisplacementFoldingTexture, waterNormalTexture, waterVAO, 300, wireframe, _previousRenderPass);
	}
}

void Ocean::setCompute(bool _useCompute)
{
	useCompute = _useCompute;
}

void Ocean::setTesselation(bool _usetesselation)
{
	useTesselation = _usetesselation;
}

void Ocean::setWireframe(bool _wireframe)
{
	wireframe = _wireframe;
}

bool Ocean::isCompute() const
{
	return useCompute;
}

bool Ocean::isTesselation() const
{
	return useTesselation;
}

bool Ocean::isWireframe() const
{
	return wireframe;
}

void Ocean::createWaterAttachments(unsigned int _resolution)
{
	GLuint textures[] = { tildeH0kTexture, tildeH0minusKTexture, tildeHktDxTexture, tildeHktDyTexture, tildeHktDzTexture, pingPongTextureA, pingPongTextureB, pingPongTextureC,
		twiddleIndicesTexture, waterDisplacementFoldingTexture, waterNormalTexture };
	glDeleteTextures(sizeof(textures) / sizeof(GLuint), textures);

	glBindFramebuffer(GL_FRAMEBUFFER, fftFbo);

	glGenTextures(1, &tildeH0kTexture);
	glBindTexture(GL_TEXTURE_2D, tildeH0kTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, _resolution, _resolution, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tildeH0kTexture, 0);

	glGenTextures(1, &tildeH0minusKTexture);
	glBindTexture(GL_TEXTURE_2D, tildeH0minusKTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, _resolution, _resolution, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, tildeH0minusKTexture, 0);

	glGenTextures(1, &tildeHktDxTexture);
	glBindTexture(GL_TEXTURE_2D, tildeHktDxTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, _resolution, _resolution, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, tildeHktDxTexture, 0);

	glGenTextures(1, &tildeHktDyTexture);
	glBindTexture(GL_TEXTURE_2D, tildeHktDyTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, _resolution, _resolution, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, tildeHktDyTexture, 0);

	glGenTextures(1, &tildeHktDzTexture);
	glBindTexture(GL_TEXTURE_2D, tildeHktDzTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, _resolution, _resolution, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, tildeHktDzTexture, 0);

	glGenTextures(1, &pingPongTextureA);
	glBindTexture(GL_TEXTURE_2D, pingPongTextureA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, _resolution, _resolution, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, pingPongTextureA, 0);

	glGenTextures(1, &pingPongTextureB);
	glBindTexture(GL_TEXTURE_2D, pingPongTextureB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, _resolution, _resolution, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, pingPongTextureB, 0);

	glGenTextures(1, &pingPongTextureC);
	glBindTexture(GL_TEXTURE_2D, pingPongTextureC);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, _resolution, _resolution, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT7, GL_TEXTURE_2D, pingPongTextureC, 0);


	glBindFramebuffer(GL_FRAMEBUFFER, twiddleIndicesFbo);

	glGenTextures(1, &twiddleIndicesTexture);
	glBindTexture(GL_TEXTURE_2D, twiddleIndicesTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, glm::log2(_resolution), _resolution, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, twiddleIndicesTexture, 0);


	glBindFramebuffer(GL_FRAMEBUFFER, waterFbo);

	glGenTextures(1, &waterDisplacementFoldingTexture);
	glBindTexture(GL_TEXTURE_2D, waterDisplacementFoldingTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _resolution, _resolution, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, waterDisplacementFoldingTexture, 0);

	glGenTextures(1, &waterNormalTexture);
	glBindTexture(GL_TEXTURE_2D, waterNormalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _resolution, _resolution, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, waterNormalTexture, 0);
}

void Ocean::precomputeFftTextures(const Water & _water, RenderPass **_previousRenderPass)
{
	if (useCompute)
	{
		// tildeh0k/minusk
		{
			std::shared_ptr<Texture> noise0;
			std::shared_ptr<Texture> noise1;
			std::shared_ptr<Texture> noise2;
			std::shared_ptr<Texture> noise3;

			// consider keeping these textures permanently in memory
			switch (_water.simulationResolution)
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

			tildeH0kCompShader->bind();

			uSimulationResolutionH0C.set(_water.simulationResolution);
			uWorldSizeH0C.set(_water.worldSize);
			uWaveAmplitudeH0C.set(_water.waveAmplitude);
			uWindDirectionH0C.set(_water.normalizedWindDirection);
			uWindSpeedH0C.set(_water.windSpeed);
			uWaveSuppressionExpH0C.set(_water.waveSuppressionExponent);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, noise0->getId());
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, noise1->getId());
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, noise2->getId());
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, noise3->getId());

			glBindImageTexture(0, tildeH0kTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
			glBindImageTexture(1, tildeH0minusKTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
			glDispatchCompute(_water.simulationResolution / 8, _water.simulationResolution / 8, 1);
			glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
		}

		// butterfly precompute
		{
			std::unique_ptr<uint32_t[]> bitReversedIndices = std::make_unique<uint32_t[]>(static_cast<size_t>(_water.simulationResolution));

			for (std::uint32_t i = 0; i < _water.simulationResolution; ++i)
			{
				std::uint32_t x = glm::bitfieldReverse(i);
				x = glm::bitfieldRotateRight(x, glm::log2(_water.simulationResolution));
				bitReversedIndices[static_cast<size_t>(i)] = x;
			}

			butterflyPrecomputeCompShader->bind();
			uSimulationResolutionBPC.set(_water.simulationResolution);
			for (size_t i = 0; i < static_cast<size_t>(_water.simulationResolution); ++i)
			{
				butterflyPrecomputeCompShader->setUniform(uJBPC[i], (int)bitReversedIndices[i]);
			}

			glBindImageTexture(0, twiddleIndicesTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
			glDispatchCompute(glm::log2(_water.simulationResolution), _water.simulationResolution / 8, 1);
			glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
		}
	}
	else
	{
		tildeH0kRenderPass->render(_water, _previousRenderPass);
		butterflyPrecomputeRenderPass->render(_water, _previousRenderPass);
	}
}

void Ocean::computeFft(const Water & _water, RenderPass **_previousRenderPass)
{
	if (useCompute)
	{
		// tildehkt
		{
			tildeHktCompShader->bind();
			uSimulationResolutionHTC.set(_water.simulationResolution);
			uWorldSizeHTC.set(_water.worldSize);
			uTimeHTC.set((float)Engine::getTime() * _water.timeScale);

			glBindImageTexture(0, tildeHktDxTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG16F);
			glBindImageTexture(1, tildeHktDyTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG16F);
			glBindImageTexture(2, tildeHktDzTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG16F);
			glBindImageTexture(3, tildeH0kTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG16F);
			glBindImageTexture(4, tildeH0minusKTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG16F);
			glDispatchCompute(_water.simulationResolution / 8, _water.simulationResolution / 8, 1);
			glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
		}

		// butterfly computation/ inversion
		{
			butterflyComputeCompShader->bind();


			glBindImageTexture(3, pingPongTextureA, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG16F);
			glBindImageTexture(4, pingPongTextureB, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG16F);
			glBindImageTexture(5, pingPongTextureC, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG16F);
			glBindImageTexture(6, twiddleIndicesTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);

			int pingpong = 0;

			for (int i = 0; i < 2; ++i)
			{
				uDirectionBCC.set(i);

				for (int j = 0; unsigned int(j) < glm::log2(_water.simulationResolution); ++j)
				{
					uStageBCC.set(j);
					uPingPongBCC.set(pingpong);

					glDispatchCompute(_water.simulationResolution / 8, _water.simulationResolution / 8, 1);
					glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

					pingpong = 1 - pingpong;
				}
			}

			// inverse/permute
			{
				inversePermuteCompShader->bind();
				uSimulationResolutionIPC.set(_water.simulationResolution);
				uPingPongIPC.set(pingpong);
				uChoppinessIPC.set(-_water.waveChoppiness);

				glBindImageTexture(6, waterDisplacementFoldingTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);

				glDispatchCompute(_water.simulationResolution / 8, _water.simulationResolution / 8, 1);
				glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

				// generate mips
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, waterDisplacementFoldingTexture);
				glGenerateMipmap(GL_TEXTURE_2D);
			}

			// normal
			{
				waterNormalCompShader->bind();

				uNormalStrengthNC.set(_water.normalStrength);

				glBindImageTexture(0, waterNormalTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);

				glDispatchCompute(_water.simulationResolution / 8, _water.simulationResolution / 8, 1);
				glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

				// generate mips
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, waterNormalTexture);
				glGenerateMipmap(GL_TEXTURE_2D);
			}
		}
	}
	else
	{
		GLuint readTextures[] = { tildeHktDxTexture, tildeHktDyTexture, tildeHktDzTexture , pingPongTextureA, pingPongTextureB, pingPongTextureC };

		tildeHktRenderPass->render(_water, tildeH0kTexture, tildeH0minusKTexture, _previousRenderPass);
		butterflyComputeRenderPass->render(_water, twiddleIndicesTexture, readTextures, _previousRenderPass);
		inversePermuteRenderPass->render(_water, readTextures, waterDisplacementFoldingTexture, _previousRenderPass);
		oceanNormalRenderPass->render(_water, waterDisplacementFoldingTexture, waterNormalTexture, _previousRenderPass);
	}
}

void Ocean::createWaterPlane()
{
	waterPlaneCreated = true;

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

	glGenVertexArrays(1, &waterVAO);
	glGenBuffers(1, &waterVBO);
	glGenBuffers(1, &waterEBO);

	glBindVertexArray(waterVAO);

	glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, waterEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
}

void Ocean::createTileRings()
{
	tileRingsCreated = true;

	// init terrain tile rings
	float tileWidth = 16.0f;
	int ringWidth = 16;
	for (int i = 0; i < 6; ++i)
	{
		int innerWidth = (i == 0) ? 0 : ringWidth / 2;
		int outerWidth = ringWidth;
		tileRings[i] = new TileRing(innerWidth, outerWidth, tileWidth);
		tileWidth *= 2.0f;
	}
}
