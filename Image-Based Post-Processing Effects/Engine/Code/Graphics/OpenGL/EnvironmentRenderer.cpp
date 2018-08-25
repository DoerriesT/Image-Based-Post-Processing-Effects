#include "EnvironmentRenderer.h"
#include <glm\gtc\matrix_transform.hpp>
#include <glm\ext.hpp>
#include ".\..\..\Graphics\EnvironmentProbe.h"
#include ".\..\..\Graphics\Mesh.h"
#include ".\..\..\Graphics\Texture.h"
#include "ShaderProgram.h"
#include "Level.h"

EnvironmentRenderer::EnvironmentRenderer()
{
	rotations[0] = glm::mat3(glm::rotate(glm::radians(-90.0f), glm::vec3(0.0, 1.0, 0.0))) * glm::mat3(glm::rotate(glm::radians(180.0f), glm::vec3(0.0, 0.0, -1.0)));
	rotations[1] = glm::mat3(glm::rotate(glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0))) * glm::mat3(glm::rotate(glm::radians(180.0f), glm::vec3(0.0, 0.0, 1.0)));
	rotations[2] = glm::mat3(glm::rotate(glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0)));
	rotations[3] = glm::mat3(glm::rotate(glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0)));
	rotations[4] = glm::mat3(glm::rotate(glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0))) * glm::mat3(glm::rotate(glm::radians(180.0f), glm::vec3(0.0, 0.0, -1.0)));
	rotations[5] = glm::mat3(glm::rotate(glm::radians(0.0f), glm::vec3(0.0, 1.0, 0.0))) * glm::mat3(glm::rotate(glm::radians(180.0f), glm::vec3(0.0, 0.0, 1.0)));
}

EnvironmentRenderer::~EnvironmentRenderer()
{
	// delete fbo etc
}

void EnvironmentRenderer::init()
{
	atmosphereShader = ShaderProgram::createShaderProgram("Resources/Shaders/Environment/environmentProjection.vert", "Resources/Shaders/Environment/atmosphere.frag");
	blitShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Shared/blit.frag");
	reflectanceOctShader = ShaderProgram::createShaderProgram("Resources/Shaders/Environment/reflectance.comp");
	irradianceOctShader = ShaderProgram::createShaderProgram("Resources/Shaders/Environment/irradiance.comp");
	
	// uniforms
	uScreenTextureBlit = blitShader->createUniform("uScreenTexture");

	uImageSizeIO.create(irradianceOctShader);

	uEnvironmentResolutionRO.create(reflectanceOctShader);
	uRoughnessRO.create(reflectanceOctShader);
	uImageSizeRO.create(reflectanceOctShader);

	uRotationA.create(atmosphereShader);
	uInverseProjectionA.create(atmosphereShader);
	uLightDirA.create(atmosphereShader);
	uRayleighBrightnessA.create(atmosphereShader);
	uMieBrightnessA.create(atmosphereShader);
	uMieDistributionA.create(atmosphereShader);
	uSpotBrightnessA.create(atmosphereShader);
	uSurfaceHeightA.create(atmosphereShader);
	uStepCountA.create(atmosphereShader);
	uIntensityA.create(atmosphereShader);
	uScatterStrengthA.create(atmosphereShader);
	uRayleighStrengthA.create(atmosphereShader);
	uMieStrengthA.create(atmosphereShader);
	uRayleighCollectionPowerA.create(atmosphereShader);
	uMieCollectionPowerA.create(atmosphereShader);

	const glm::mat4 invProjection = glm::inverse(glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f));
	atmosphereShader->bind();
	uInverseProjectionA.set(invProjection);

	

	glGenFramebuffers(1, &environmentFbo);
	glGenFramebuffers(1, &convolutionFbo);

	glBindFramebuffer(GL_FRAMEBUFFER, environmentFbo);

	glGenTextures(1, &environmentMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMap);
	for (int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, ENVIRONMENT_MAP_SIZE, ENVIRONMENT_MAP_SIZE, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	for (int i = 0; i < 6; ++i)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, environmentMap, 0);
	}

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void EnvironmentRenderer::updateCubeSide(unsigned int _side, GLuint _source)
{
	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, environmentFbo);
	glViewport(0, 0, ENVIRONMENT_MAP_SIZE, ENVIRONMENT_MAP_SIZE);
	glDrawBuffer(GL_COLOR_ATTACHMENT0 + _side);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _source);
	blitShader->bind();
	blitShader->setUniform(uScreenTextureBlit, 0); 

	fullscreenTriangle->getSubMesh()->render();
	//glDrawArrays(GL_TRIANGLES, 0, 3);
}

void EnvironmentRenderer::generateMipmaps()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMap);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

void EnvironmentRenderer::calculateReflectance(const std::shared_ptr<EnvironmentProbe>  &_environmentProbe)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMap);

	unsigned int maxMipLevels = 5;// glm::log2(EnvironmentProbe::REFLECTANCE_RESOLUTION);

	reflectanceOctShader->bind();
	uEnvironmentResolutionRO.set(ENVIRONMENT_MAP_SIZE);

	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		unsigned int mipRes = unsigned int(1024 * std::pow(0.5f, mip));

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		uRoughnessRO.set(roughness);
		uImageSizeRO.set(glm::vec2(mipRes));
		glBindImageTexture(0, _environmentProbe->getReflectanceMap()->getId(), mip, GL_FALSE, 0, GL_WRITE_ONLY, GL_R11F_G11F_B10F);
		glDispatchCompute(mipRes / 8, mipRes / 8, 1);
	}
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void EnvironmentRenderer::calculateIrradiance(const std::shared_ptr<EnvironmentProbe>  &_environmentProbe)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, environmentMap);

	irradianceOctShader->bind();
	uImageSizeIO.set(glm::vec2(EnvironmentProbe::IRRADIANCE_RESOLUTION));
	glBindImageTexture(0, _environmentProbe->getIrradianceMap()->getId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R11F_G11F_B10F);
	glDispatchCompute(EnvironmentProbe::IRRADIANCE_RESOLUTION / 8, EnvironmentProbe::IRRADIANCE_RESOLUTION / 8, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

std::shared_ptr<Texture> EnvironmentRenderer::calculateAtmosphere(const AtmosphereParams &_atmosphereParams)
{
	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	glBindFramebuffer(GL_FRAMEBUFFER, convolutionFbo);

	GLuint atmosphereMap;

	glGenTextures(1, &atmosphereMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, atmosphereMap);
	for (int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, ATMOSPHERE_MAP_SIZE, ATMOSPHERE_MAP_SIZE, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	for (int i = 0; i < 6; ++i)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, atmosphereMap, 0);
	}

	atmosphereShader->bind();

	uLightDirA.set(_atmosphereParams.lightDir);
	uRayleighBrightnessA.set(_atmosphereParams.rayleighBrightness);
	uMieBrightnessA.set(_atmosphereParams.mieBrightness);
	uMieDistributionA.set(_atmosphereParams.mieDistribution);
	uSpotBrightnessA.set(_atmosphereParams.spotBrightness);
	uSurfaceHeightA.set(_atmosphereParams.surfaceHeight);
	uStepCountA.set(_atmosphereParams.stepCount);
	uIntensityA.set(_atmosphereParams.intensity);
	uScatterStrengthA.set(_atmosphereParams.scatterStrength);
	uRayleighStrengthA.set(_atmosphereParams.rayleighStrength);
	uMieStrengthA.set(_atmosphereParams.mieStrength);
	uRayleighCollectionPowerA.set(_atmosphereParams.rayleighCollectionPower);
	uMieCollectionPowerA.set(_atmosphereParams.mieCollectionPower);

	glViewport(0, 0, ATMOSPHERE_MAP_SIZE, ATMOSPHERE_MAP_SIZE);

	for (unsigned int i = 0; i < 6; ++i)
	{
		uRotationA.set(rotations[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, atmosphereMap, 0);
		fullscreenTriangle->getSubMesh()->render();
		//glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	return Texture::createTexture(atmosphereMap, GL_TEXTURE_CUBE_MAP);
}

GLuint EnvironmentRenderer::getEnvironmentMap() const
{
	return environmentMap;
}