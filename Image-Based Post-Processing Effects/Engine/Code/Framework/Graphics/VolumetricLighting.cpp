#include "VolumetricLighting.h"
#include "RenderData.h"
#include "Level.h"

VolumetricLighting::VolumetricLighting(unsigned int _width, unsigned int _height)
	:width(_width), height(_height)
{
}

VolumetricLighting::~VolumetricLighting()
{
	GLuint textures[] = { lightVolumeTexture , depthStencilTexture };
	glDeleteTextures(sizeof(textures) / sizeof(GLuint), textures);

	glDeleteFramebuffers(1, &lightVolumeFbo);
}

void VolumetricLighting::init()
{
	lightVolumeShader = ShaderProgram::createShaderProgram("Resources/Shaders/Renderer/lightVolume.vert", "Resources/Shaders/Renderer/lightVolume.frag", "Resources/Shaders/Renderer/lightVolume.tessc", "Resources/Shaders/Renderer/lightVolume.tesse");
	phaseLUTShader = ShaderProgram::createShaderProgram("Resources/Shaders/Renderer/phaseLookup.comp");

	// light volume
	uDisplacementTextureLV.create(lightVolumeShader);
	uInvLightViewProjectionLV.create(lightVolumeShader);
	uViewProjectionLV.create(lightVolumeShader);
	uPhaseLUTLV.create(lightVolumeShader);
	uCamPosLV.create(lightVolumeShader);
	uLightIntensitysLV.create(lightVolumeShader);
	uSigmaExtinctionLV.create(lightVolumeShader);
	uScatterPowerLV.create(lightVolumeShader);
	uLightDirLV.create(lightVolumeShader);

	// phase lookup
	uNumPhaseTermsPL.create(phaseLUTShader);
	for (int i = 0; i < 4; ++i)
	{
		uPhaseParamsPL.push_back(phaseLUTShader->createUniform(std::string("uPhaseParams") + "[" + std::to_string(i) + "]"));
		uPhaseFuncPL.push_back(phaseLUTShader->createUniform(std::string("uPhaseFunc") + "[" + std::to_string(i) + "]"));
	}

	// create FBO
	glGenFramebuffers(1, &lightVolumeFbo);

	createAttachments(width, height);
	computePhaseLUT();
}

void VolumetricLighting::render(GLuint _depthTexture, const RenderData &_renderData, const std::shared_ptr<Level> &_level)
{
	if (!meshCreated)
	{
		createLightVolumeMesh();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, lightVolumeFbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, _depthTexture, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	const float SCATTER_EPSILON = 0.000001f;
	glm::vec3 total_scatter = glm::vec3(SCATTER_EPSILON, SCATTER_EPSILON, SCATTER_EPSILON);

	for (uint32_t p = 0; p < currentMediumDescription.uNumPhaseTerms; ++p)
	{
		total_scatter += currentMediumDescription.phaseTerms[p].vDensity;
	}
	glm::vec3 absorption = currentMediumDescription.vAbsorption;
	glm::vec3 vScatterPower;
	vScatterPower.x = 1.0f - exp(-total_scatter.x);
	vScatterPower.y = 1.0f - exp(-total_scatter.y);
	vScatterPower.z = 1.0f - exp(-total_scatter.z);
	glm::vec3 vSigmaExtinction = total_scatter + absorption;


	lightVolumeShader->bind();
	uDisplacementTextureLV.set(0);
	uInvLightViewProjectionLV.set(glm::inverse(_level->lights.directionalLights[0]->getViewProjectionMatrix()));
	uViewProjectionLV.set(_renderData.viewProjectionMatrix);

	uPhaseLUTLV.set(1);
	uCamPosLV.set(_renderData.cameraPosition);
	uLightIntensitysLV.set(_level->lights.directionalLights[0]->getColor() * 25000.0f);
	uSigmaExtinctionLV.set(vSigmaExtinction);
	uScatterPowerLV.set(vScatterPower);
	uLightDirLV.set(_level->lights.directionalLights[0]->getDirection());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _level->lights.directionalLights[0]->getShadowMap());

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, phaseLUT);

	glEnable(GL_STENCIL_TEST);
	glStencilFuncSeparate(GL_FRONT, GL_ALWAYS, 0xFF, 0xFF);
	glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_INCR, GL_KEEP);
	glStencilFuncSeparate(GL_BACK, GL_ALWAYS, 0xFF, 0xFF);
	glStencilOpSeparate(GL_BACK, GL_KEEP, GL_DECR, GL_KEEP);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);

	glBindVertexArray(lightVolumeVAO);
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElements(GL_PATCHES, 64 * 64 * 4, GL_UNSIGNED_INT, NULL);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
}

void VolumetricLighting::resize(unsigned int _width, unsigned int _height)
{
	width = _width;
	height = _height;
	createAttachments(width, height);
}

GLuint VolumetricLighting::getLightVolumeTexture() const
{
	return lightVolumeTexture;
}

void VolumetricLighting::computePhaseLUT()
{
	glDeleteTextures(1, &phaseLUT);
	glGenTextures(1, &phaseLUT);

	glBindTexture(GL_TEXTURE_2D, phaseLUT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1, 512, 0, GL_RGBA, GL_HALF_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	phaseLUTShader->bind();

	const float SCATTER_PARAM_SCALE = 0.0001f;

	uint32_t t = 0;

	currentMediumDescription.phaseTerms[t].ePhaseFunc = 1;
	currentMediumDescription.phaseTerms[t].vDensity = (10.00f * SCATTER_PARAM_SCALE * glm::vec3(0.596f, 1.324f, 3.310f));
	t++;

	int mediumType = 0;

	switch (mediumType)
	{
	default:
	case 0:
		currentMediumDescription.phaseTerms[t].ePhaseFunc = 2;
		currentMediumDescription.phaseTerms[t].vDensity = (10.00f * SCATTER_PARAM_SCALE * glm::vec3(1.00f, 1.00f, 1.00f));
		currentMediumDescription.phaseTerms[t].fEccentricity = 0.85f;
		t++;
		currentMediumDescription.vAbsorption = (5.0f * SCATTER_PARAM_SCALE * glm::vec3(1, 1, 1));
		break;

	case 1:
		currentMediumDescription.phaseTerms[t].ePhaseFunc = 2;
		currentMediumDescription.phaseTerms[t].vDensity = (15.00f * SCATTER_PARAM_SCALE * glm::vec3(1.00f, 1.00f, 1.00f));
		currentMediumDescription.phaseTerms[t].fEccentricity = 0.60f;
		t++;
		currentMediumDescription.vAbsorption = (25.0f * SCATTER_PARAM_SCALE * glm::vec3(1, 1, 1));
		break;

	case 2:
		currentMediumDescription.phaseTerms[t].ePhaseFunc = 3;
		currentMediumDescription.phaseTerms[t].vDensity = (20.00f * SCATTER_PARAM_SCALE * glm::vec3(1.00f, 1.00f, 1.00f));
		t++;
		currentMediumDescription.vAbsorption = (25.0f * SCATTER_PARAM_SCALE * glm::vec3(1, 1, 1));
		break;

	case 3:
		currentMediumDescription.phaseTerms[t].ePhaseFunc = 4;
		currentMediumDescription.phaseTerms[t].vDensity = (30.00f * SCATTER_PARAM_SCALE * glm::vec3(1.00f, 1.00f, 1.00f));
		t++;
		currentMediumDescription.vAbsorption = (50.0f * SCATTER_PARAM_SCALE * glm::vec3(1, 1, 1));
		break;
	}

	uNumPhaseTermsPL.set(t);
	for (unsigned int i = 0; i < t; ++i)
	{
		phaseLUTShader->setUniform(uPhaseParamsPL[i], glm::vec4(currentMediumDescription.phaseTerms[i].vDensity, currentMediumDescription.phaseTerms[i].fEccentricity));
		phaseLUTShader->setUniform(uPhaseFuncPL[i], currentMediumDescription.phaseTerms[i].ePhaseFunc);
	}

	glBindImageTexture(0, phaseLUT, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	glDispatchCompute(8, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void VolumetricLighting::createLightVolumeMesh()
{
	meshCreated = true;

	const unsigned int SIZE = 64;
	GLuint *indices = new GLuint[SIZE * SIZE * 4];
	int currentIndex = 0;
	for (unsigned int row = 0; row < SIZE; ++row)
	{
		for (unsigned int column = 0; column < SIZE; ++column)
		{
			indices[currentIndex++] = (row * (SIZE + 1) + column);
			indices[currentIndex++] = (row * (SIZE + 1) + column) + 1;
			indices[currentIndex++] = ((row + 1) * (SIZE + 1) + column) + 1;
			indices[currentIndex++] = ((row + 1) * (SIZE + 1) + column);
		}
	}

	glm::vec2 *positions = new glm::vec2[(SIZE + 1) * (SIZE + 1)];
	for (unsigned int y = 0; y < SIZE + 1; ++y)
	{
		for (unsigned int x = 0; x < SIZE + 1; ++x)
		{
			positions[y * (SIZE + 1) + x] = glm::vec2(x / float(SIZE + 1), y / float(SIZE + 1));
		}
	}

	// create buffers/arrays
	glGenVertexArrays(1, &lightVolumeVAO);
	glGenBuffers(1, &lightVolumeVBO);
	glGenBuffers(1, &lightVolumeEBO);
	glBindVertexArray(lightVolumeVAO);
	//glBindBuffer(GL_ARRAY_BUFFER, _VBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * (_size + 1) * (_size + 1), positions, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightVolumeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * SIZE * SIZE * 4, indices, GL_STATIC_DRAW);

	// patch position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);

	glBindVertexArray(0);

	delete[] indices;
	delete[] positions;
}

void VolumetricLighting::createAttachments(unsigned int _width, unsigned int _height)
{
	GLuint textures[] = { lightVolumeTexture , depthStencilTexture };
	glDeleteTextures(sizeof(textures) / sizeof(GLuint), textures);

	glBindFramebuffer(GL_FRAMEBUFFER, lightVolumeFbo);

	glGenTextures(1, &lightVolumeTexture);
	glBindTexture(GL_TEXTURE_2D, lightVolumeTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, _width, _height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightVolumeTexture, 0);

	glGenTextures(1, &depthStencilTexture);
	glBindTexture(GL_TEXTURE_2D, depthStencilTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, _width, _height, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencilTexture, 0);
}