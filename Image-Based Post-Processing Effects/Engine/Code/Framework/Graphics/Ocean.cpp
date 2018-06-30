#include "Ocean.h"
#include".\..\..\Graphics\Texture.h"
#include <glm\ext.hpp>
#include "Engine.h"
#include "TileRing.h"
#include ".\..\..\Graphics\Mesh.h"
#include "RenderData.h"

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

void Ocean::init()
{
	// fragment
	tildeH0kShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Water/tildeH0k.frag");
	tildeHktShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Water/tildeHkt.frag");
	butterflyPrecomputeShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Water/butterflyPrecompute.frag");
	butterflyComputeShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Water/butterflyCompute.frag");
	inversePermuteShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Water/inversePermute.frag");
	waterNormalShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Water/normal.frag");
	
	// compute
	tildeH0kCompShader = ShaderProgram::createShaderProgram("Resources/Shaders/Water/tildeH0k.comp");
	tildeHktCompShader = ShaderProgram::createShaderProgram("Resources/Shaders/Water/tildeHkt.comp");
	butterflyPrecomputeCompShader = ShaderProgram::createShaderProgram("Resources/Shaders/Water/butterflyPrecompute.comp");
	butterflyComputeCompShader = ShaderProgram::createShaderProgram("Resources/Shaders/Water/butterflyCompute.comp");
	inversePermuteCompShader = ShaderProgram::createShaderProgram("Resources/Shaders/Water/inversePermute.comp");
	waterNormalCompShader = ShaderProgram::createShaderProgram("Resources/Shaders/Water/normal.comp");

	// world space grid
	waterShader = ShaderProgram::createShaderProgram("Resources/Shaders/Water/water.vert", "Resources/Shaders/Water/Water.frag");
	
	// tesselation
	waterTessShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/terrain.vert", "Resources/Shaders/Water/water.frag", "Resources/Shaders/Shared/terrain.tessc", "Resources/Shaders/Shared/terrain.tesse");

	// fragment uniforms
	{
		// tildeh0k
		uSimulationResolutionH0.create(tildeH0kShader);
		uWorldSizeH0.create(tildeH0kShader);
		uWaveAmplitudeH0.create(tildeH0kShader);
		uWindDirectionH0.create(tildeH0kShader);
		uWindSpeedH0.create(tildeH0kShader);

		// tildehkt
		uSimulationResolutionHT.create(tildeHktShader);
		uWorldSizeHT.create(tildeHktShader);
		uTimeHT.create(tildeHktShader);

		// butterfly precompute
		for (int i = 0; i < 512; ++i)
		{
			uJBP.push_back(butterflyPrecomputeShader->createUniform(std::string("uJ") + "[" + std::to_string(i) + "]"));
		}
		uSimulationResolutionBP.create(butterflyPrecomputeShader);

		// butterfly compute
		uSimulationResolutionBC.create(butterflyComputeShader);
		uStageBC.create(butterflyComputeShader);
		uStagesBC.create(butterflyComputeShader);
		uDirectionBC.create(butterflyComputeShader);

		// inverse / permute
		uSimulationResolutionIP.create(inversePermuteShader);
		uChoppinessIP.create(inversePermuteShader);

		// water normal
		uNormalStrengthN.create(waterNormalShader);
	}
	
	// compute uniforms
	{
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
	}

	// world space grid
	{
		// water
		uProjectionW.create(waterShader);
		uViewW.create(waterShader);
		uCamPosW.create(waterShader);
		uTexCoordShiftW.create(waterShader);
		uUseEnvironmentW.create(waterShader);
		uWaterLevelW.create(waterShader);
		uLightDirW.create(waterShader);
		uLightColorW.create(waterShader);
	}

	// tesselated grid
	{
		// water tesselated
		uViewProjectionWT.create(waterTessShader);
		uProjectionWT.create(waterTessShader);
		uViewWT.create(waterTessShader);
		uCamPosWT.create(waterTessShader);
		uTexCoordShiftWT.create(waterTessShader);
		uUseEnvironmentWT.create(waterTessShader);
		uWaterLevelWT.create(waterTessShader);
		uLightDirWT.create(waterTessShader);
		uLightColorWT.create(waterTessShader);
		uTileSizeWT.create(waterTessShader);
		uViewDirWT.create(waterTessShader);
		uScreenSizeWT.create(waterTessShader);
		uTesselatedTriWidthWT.create(waterTessShader);
		uTexCoordScaleWT.create(waterTessShader);
		uDisplacementScaleWT.create(waterTessShader);
	}

	// create FBO
	glGenFramebuffers(1, &fftFbo);
	glGenFramebuffers(1, &twiddleIndicesFbo);
	glGenFramebuffers(1, &waterFbo);

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void Ocean::prepareRender(const RenderData &_renderData, const std::shared_ptr<Level> &_level)
{
	bool recompute = false;

	if (currentWaterConfig.simulationResolution != _level->water.simulationResolution)
	{
		currentWaterConfig.simulationResolution = _level->water.simulationResolution;
		recompute = true;
		createWaterAttachments(currentWaterConfig.simulationResolution);
	}

	if (currentWaterConfig.normalizedWindDirection != _level->water.normalizedWindDirection ||
		currentWaterConfig.waveAmplitude != _level->water.waveAmplitude ||
		currentWaterConfig.waveSuppressionExponent != _level->water.waveSuppressionExponent ||
		currentWaterConfig.windSpeed != _level->water.windSpeed ||
		currentWaterConfig.worldSize != _level->water.worldSize)
	{
		recompute = true;
	}

	currentWaterConfig = _level->water;

	glDisable(GL_CULL_FACE);

	if (recompute)
	{
		precomputeFftTextures(currentWaterConfig);
	}

	computeFft(currentWaterConfig);
	glEnable(GL_CULL_FACE);
}

void Ocean::render(const RenderData &_renderData, const std::shared_ptr<Level> &_level)
{
	static std::shared_ptr<Texture> foamTexture = Texture::createTexture("Resources/Textures/foam.dds", true);

	if (useTesselation)
	{
		if (!tileRingsCreated)
		{
			createTileRings();
		}

		waterTessShader->bind();
		uViewProjectionWT.set(_renderData.viewProjectionMatrix);
		uProjectionWT.set(_renderData.projectionMatrix);
		uViewWT.set(_renderData.viewMatrix);
		uCamPosWT.set(_renderData.cameraPosition);
		uTexCoordShiftWT.set(glm::vec2(-1.5, 0.75) * _renderData.time * 0.25);
		uUseEnvironmentWT.set(_level->environment.environmentProbe->isValid());
		uWaterLevelWT.set(_level->water.level);
		if (_level->lights.directionalLights.empty())
		{
			uLightDirWT.set(glm::normalize(glm::vec3(1.0f, 1.0f, 0.0f)));
			uLightColorWT.set(glm::vec3(1.5f, 0.575f, 0.5f));
		}
		else
		{
			uLightDirWT.set(_level->lights.directionalLights[0]->getDirection());
			uLightColorWT.set(_level->lights.directionalLights[0]->getColor());
		}

		uViewDirWT.set(_renderData.viewDirection);
		uScreenSizeWT.set(glm::vec2(_renderData.resolution.first, _renderData.resolution.second));
		uTesselatedTriWidthWT.set(20);
		uTexCoordScaleWT.set(0.1f);
		uDisplacementScaleWT.set(1.0f);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, waterNormalTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, waterDisplacementFoldingTexture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, foamTexture->getId());
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_CUBE_MAP, _level->environment.environmentProbe->getReflectanceMap()->getId());

		uTileSizeWT.set(1.0f);

		glDisable(GL_CULL_FACE);
		if (wireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		for (int i = 0; i < 6; ++i)
		{
			uTileSizeWT.set(tileRings[i]->getTileSize());
			tileRings[i]->render();
		}
		if (wireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		glEnable(GL_CULL_FACE);
	}
	else
	{
		if (!waterPlaneCreated)
		{
			createWaterPlane();
		}

		waterShader->bind();
		uProjectionW.set(_renderData.projectionMatrix);
		uViewW.set(_renderData.viewMatrix);
		uCamPosW.set(_renderData.cameraPosition);
		uTexCoordShiftW.set(glm::vec2(-1.5, 0.75) * _renderData.time * 0.25);
		uUseEnvironmentW.set(_level->environment.environmentProbe->isValid());
		uWaterLevelW.set(_level->water.level);
		if (_level->lights.directionalLights.empty())
		{
			uLightDirW.set(glm::normalize(glm::vec3(1.0f, 1.0f, 0.0f)));
			uLightColorW.set(glm::vec3(1.5f, 0.575f, 0.5f));
		}
		else
		{
			uLightDirW.set(_level->lights.directionalLights[0]->getDirection());
			uLightColorW.set(_level->lights.directionalLights[0]->getColor());
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, waterNormalTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, waterDisplacementFoldingTexture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, foamTexture->getId());
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_CUBE_MAP, _level->environment.environmentProbe->getReflectanceMap()->getId());

		glDisable(GL_CULL_FACE);
		glBindVertexArray(waterVAO);
		if (wireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		glDrawElements(GL_TRIANGLES, (GLsizei)(300 * 300 * 6), GL_UNSIGNED_INT, 0);
		if (wireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		glEnable(GL_CULL_FACE);
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _resolution, _resolution, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tildeH0kTexture, 0);

	glGenTextures(1, &tildeH0minusKTexture);
	glBindTexture(GL_TEXTURE_2D, tildeH0minusKTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _resolution, _resolution, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, tildeH0minusKTexture, 0);

	glGenTextures(1, &tildeHktDxTexture);
	glBindTexture(GL_TEXTURE_2D, tildeHktDxTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _resolution, _resolution, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, tildeHktDxTexture, 0);

	glGenTextures(1, &tildeHktDyTexture);
	glBindTexture(GL_TEXTURE_2D, tildeHktDyTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _resolution, _resolution, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, tildeHktDyTexture, 0);

	glGenTextures(1, &tildeHktDzTexture);
	glBindTexture(GL_TEXTURE_2D, tildeHktDzTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _resolution, _resolution, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, tildeHktDzTexture, 0);

	glGenTextures(1, &pingPongTextureA);
	glBindTexture(GL_TEXTURE_2D, pingPongTextureA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _resolution, _resolution, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, pingPongTextureA, 0);

	glGenTextures(1, &pingPongTextureB);
	glBindTexture(GL_TEXTURE_2D, pingPongTextureB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _resolution, _resolution, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, pingPongTextureB, 0);

	glGenTextures(1, &pingPongTextureC);
	glBindTexture(GL_TEXTURE_2D, pingPongTextureC);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _resolution, _resolution, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT7, GL_TEXTURE_2D, pingPongTextureC, 0);


	glBindFramebuffer(GL_FRAMEBUFFER, twiddleIndicesFbo);

	glGenTextures(1, &twiddleIndicesTexture);
	glBindTexture(GL_TEXTURE_2D, twiddleIndicesTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, glm::log2(_resolution), _resolution, 0, GL_RGBA, GL_FLOAT, NULL);
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

void Ocean::precomputeFftTextures(const Water & _water)
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

	if (useCompute)
	{
		// tildeh0k/minusk
		{
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
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		}

		// butterfly precompute
		{
			std::uint32_t *bitReversedIndices = new std::uint32_t[_water.simulationResolution];

			for (std::uint32_t i = 0; i < _water.simulationResolution; ++i)
			{
				std::uint32_t x = glm::bitfieldReverse(i);
				x = glm::bitfieldRotateRight(x, glm::log2(_water.simulationResolution));
				bitReversedIndices[i] = x;
			}

			butterflyPrecomputeCompShader->bind();
			uSimulationResolutionBPC.set(_water.simulationResolution);
			for (unsigned int i = 0; i < _water.simulationResolution; ++i)
			{
				butterflyPrecomputeCompShader->setUniform(uJBPC[i], (int)bitReversedIndices[i]);
			}

			delete[] bitReversedIndices;

			glBindImageTexture(0, twiddleIndicesTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
			glDispatchCompute(glm::log2(_water.simulationResolution), _water.simulationResolution / 8, 1);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		}
	}
	else
	{
		fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

		// tildeh0k/minusk
		{
			tildeH0kShader->bind();

			uSimulationResolutionH0.set(_water.simulationResolution);
			uWorldSizeH0.set(_water.worldSize);
			uWaveAmplitudeH0.set(_water.waveAmplitude);
			uWindDirectionH0.set(_water.normalizedWindDirection);
			uWindSpeedH0.set(_water.windSpeed);
			uWaveSuppressionExpH0.set(_water.waveSuppressionExponent);



			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, noise0->getId());
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, noise1->getId());
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, noise2->getId());
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, noise3->getId());

			glBindFramebuffer(GL_FRAMEBUFFER, fftFbo);
			glViewport(0, 0, _water.simulationResolution, _water.simulationResolution);

			GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			glDrawBuffers(2, drawBuffers);

			fullscreenTriangle->getSubMesh()->render();
			//glDrawArrays(GL_TRIANGLES, 0, 3);
		}

		// butterfly precompute
		{
			std::uint32_t *bitReversedIndices = new std::uint32_t[_water.simulationResolution];

			for (std::uint32_t i = 0; i < _water.simulationResolution; ++i)
			{
				std::uint32_t x = glm::bitfieldReverse(i);
				x = glm::bitfieldRotateRight(x, glm::log2(_water.simulationResolution));
				bitReversedIndices[i] = x;
			}

			butterflyPrecomputeShader->bind();
			uSimulationResolutionBP.set(_water.simulationResolution);
			for (unsigned int i = 0; i < _water.simulationResolution; ++i)
			{
				butterflyPrecomputeShader->setUniform(uJBP[i], (int)bitReversedIndices[i]);
			}

			delete[] bitReversedIndices;

			glBindFramebuffer(GL_FRAMEBUFFER, twiddleIndicesFbo);
			glViewport(0, 0, glm::log2(_water.simulationResolution), _water.simulationResolution);

			fullscreenTriangle->getSubMesh()->render();
			//glDrawArrays(GL_TRIANGLES, 0, 3);
		}
	}
}

void Ocean::computeFft(const Water & _water)
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
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
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
					glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

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
				glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

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
				glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

				// generate mips
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, waterNormalTexture);
				glGenerateMipmap(GL_TEXTURE_2D);
			}
		}
	}
	else
	{
		fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();
		// tildehkt
		{
			tildeHktShader->bind();
			uSimulationResolutionHT.set(_water.simulationResolution);
			uWorldSizeHT.set(_water.worldSize);
			uTimeHT.set((float)Engine::getTime() * _water.timeScale);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tildeH0kTexture);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, tildeH0minusKTexture);

			glBindFramebuffer(GL_FRAMEBUFFER, fftFbo);
			glViewport(0, 0, _water.simulationResolution, _water.simulationResolution);

			GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
			glDrawBuffers(3, drawBuffers);

			fullscreenTriangle->getSubMesh()->render();
			//glDrawArrays(GL_TRIANGLES, 0, 3);
		}

		// butterfly computation/ inversion
		{
			butterflyComputeShader->bind();
			uSimulationResolutionBC.set(_water.simulationResolution);
			uStagesBC.set(glm::log2(_water.simulationResolution));

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, twiddleIndicesTexture);
			glActiveTexture(GL_TEXTURE1);

			GLenum pingPongDrawBuffers[] = { GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7 };
			GLenum sourceDrawBuffers[] = { GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
			GLuint pingPongReadBuffers[] = { pingPongTextureA, pingPongTextureB, pingPongTextureC };
			GLuint sourceReadBuffers[] = { tildeHktDxTexture, tildeHktDyTexture, tildeHktDzTexture };

			GLenum *drawBuffers[] = { pingPongDrawBuffers, sourceDrawBuffers };
			GLuint *inputTextures[] = { sourceReadBuffers, pingPongReadBuffers };
			unsigned int drawBuffer = 0;

			for (int i = 0; i < 2; ++i)
			{
				uDirectionBC.set(i);

				for (int j = 0; unsigned int(j) < glm::log2(_water.simulationResolution); ++j)
				{
					uStageBC.set(j);
					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, inputTextures[drawBuffer][0]);
					glActiveTexture(GL_TEXTURE2);
					glBindTexture(GL_TEXTURE_2D, inputTextures[drawBuffer][1]);
					glActiveTexture(GL_TEXTURE3);
					glBindTexture(GL_TEXTURE_2D, inputTextures[drawBuffer][2]);
					glDrawBuffers(3, drawBuffers[drawBuffer]);

					fullscreenTriangle->getSubMesh()->render();
					//glDrawArrays(GL_TRIANGLES, 0, 3);
					drawBuffer = 1 - drawBuffer;
				}
			}

			// inverse/permute
			{
				glBindFramebuffer(GL_FRAMEBUFFER, waterFbo);
				glViewport(0, 0, _water.simulationResolution, _water.simulationResolution);

				inversePermuteShader->bind();
				uSimulationResolutionIP.set(_water.simulationResolution);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, inputTextures[drawBuffer][0]);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, inputTextures[drawBuffer][1]);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, inputTextures[drawBuffer][2]);

				glDrawBuffer(GL_COLOR_ATTACHMENT0);

				fullscreenTriangle->getSubMesh()->render();
				//glDrawArrays(GL_TRIANGLES, 0, 3);

				// generate mips
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, waterDisplacementFoldingTexture);
				glGenerateMipmap(GL_TEXTURE_2D);
			}

			// normal
			{
				waterNormalShader->bind();

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, waterDisplacementFoldingTexture);

				glDrawBuffer(GL_COLOR_ATTACHMENT1);

				fullscreenTriangle->getSubMesh()->render();
				//glDrawArrays(GL_TRIANGLES, 0, 3);

				// generate mips
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, waterNormalTexture);
				glGenerateMipmap(GL_TEXTURE_2D);
			}
		}
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
