#include "LightPropagationVolumes.h"
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\RenderPass\LPV\RSMRenderPass.h"
#include "Graphics\OpenGL\RenderPass\LPV\LightInjectionRenderPass.h"
#include "Graphics\OpenGL\RenderPass\LPV\LightPropagationRenderPass.h"
#include "Graphics\OpenGL\RenderData.h"
#include "Level.h"
#include "Graphics\Scene.h"
#include <glm\mat4x4.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\ext.hpp>
#include "Graphics\Volume.h"

size_t RSM_SIZE = 512;
size_t VOLUME_SIZE = 64;

void LightPropagationVolumes::init()
{
	glCreateFramebuffers(1, &m_propagationFbo);
	glCreateFramebuffers(1, &m_rsmFbo);

	m_rsmRenderPass = std::make_unique<RSMRenderPass>(m_rsmFbo, static_cast<unsigned int>(RSM_SIZE), static_cast<unsigned int>(RSM_SIZE));
	m_lightInjectionRenderPass = std::make_unique<LightInjectionRenderPass>(m_propagationFbo, static_cast<unsigned int>(VOLUME_SIZE * VOLUME_SIZE), static_cast<unsigned int>(VOLUME_SIZE));
	m_geometryInjectionRenderPass = std::make_unique<GeometryInjectionRenderPass>(m_propagationFbo, static_cast<unsigned int>(VOLUME_SIZE * VOLUME_SIZE), static_cast<unsigned int>(VOLUME_SIZE));
	m_lightPropagationRenderPass = std::make_unique<LightPropagationRenderPass>(m_propagationFbo, static_cast<unsigned int>(VOLUME_SIZE * VOLUME_SIZE), static_cast<unsigned int>(VOLUME_SIZE));

	// RSM textures
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_rsmFbo);

		glGenTextures(1, &m_rsmDepth);
		glBindTexture(GL_TEXTURE_2D, m_rsmDepth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, static_cast<GLsizei>(RSM_SIZE), static_cast<GLsizei>(RSM_SIZE), 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_rsmDepth, 0);

		glGenTextures(1, &m_rsmFlux);
		glBindTexture(GL_TEXTURE_2D, m_rsmFlux);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, static_cast<GLsizei>(RSM_SIZE), static_cast<GLsizei>(RSM_SIZE), 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_rsmFlux, 0);

		glGenTextures(1, &m_rsmNormal);
		glBindTexture(GL_TEXTURE_2D, m_rsmNormal);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, static_cast<GLsizei>(RSM_SIZE), static_cast<GLsizei>(RSM_SIZE), 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_rsmNormal, 0);
	}

	// 2d textures
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_propagationFbo);

		glGenTextures(1, &m_geometry2DVolume);
		glBindTexture(GL_TEXTURE_2D, m_geometry2DVolume);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, static_cast<GLsizei>(VOLUME_SIZE * VOLUME_SIZE), static_cast<GLsizei>(VOLUME_SIZE), 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_geometry2DVolume, 0);

		glGenTextures(1, &m_propagation2DAccumVolumeRed);
		glBindTexture(GL_TEXTURE_2D, m_propagation2DAccumVolumeRed);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, static_cast<GLsizei>(VOLUME_SIZE * VOLUME_SIZE), static_cast<GLsizei>(VOLUME_SIZE), 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenTextures(1, &m_propagation2DAccumVolumeGreen);
		glBindTexture(GL_TEXTURE_2D, m_propagation2DAccumVolumeGreen);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, static_cast<GLsizei>(VOLUME_SIZE * VOLUME_SIZE), static_cast<GLsizei>(VOLUME_SIZE), 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenTextures(1, &m_propagation2DAccumVolumeBlue);
		glBindTexture(GL_TEXTURE_2D, m_propagation2DAccumVolumeBlue);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, static_cast<GLsizei>(VOLUME_SIZE * VOLUME_SIZE), static_cast<GLsizei>(VOLUME_SIZE), 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenTextures(1, &m_propagation2DVolumeRed0);
		glBindTexture(GL_TEXTURE_2D, m_propagation2DVolumeRed0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, static_cast<GLsizei>(VOLUME_SIZE * VOLUME_SIZE), static_cast<GLsizei>(VOLUME_SIZE), 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_propagation2DVolumeRed0, 0);

		glGenTextures(1, &m_propagation2DVolumeGreen0);
		glBindTexture(GL_TEXTURE_2D, m_propagation2DVolumeGreen0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, static_cast<GLsizei>(VOLUME_SIZE * VOLUME_SIZE), static_cast<GLsizei>(VOLUME_SIZE), 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_propagation2DVolumeGreen0, 0);

		glGenTextures(1, &m_propagation2DVolumeBlue0);
		glBindTexture(GL_TEXTURE_2D, m_propagation2DVolumeBlue0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, static_cast<GLsizei>(VOLUME_SIZE * VOLUME_SIZE), static_cast<GLsizei>(VOLUME_SIZE), 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_propagation2DVolumeBlue0, 0);

		glGenTextures(1, &m_propagation2DVolumeRed1);
		glBindTexture(GL_TEXTURE_2D, m_propagation2DVolumeRed1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, static_cast<GLsizei>(VOLUME_SIZE * VOLUME_SIZE), static_cast<GLsizei>(VOLUME_SIZE), 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, m_propagation2DVolumeRed1, 0);

		glGenTextures(1, &m_propagation2DVolumeGreen1);
		glBindTexture(GL_TEXTURE_2D, m_propagation2DVolumeGreen1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, static_cast<GLsizei>(VOLUME_SIZE * VOLUME_SIZE), static_cast<GLsizei>(VOLUME_SIZE), 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, m_propagation2DVolumeGreen1, 0);

		glGenTextures(1, &m_propagation2DVolumeBlue1);
		glBindTexture(GL_TEXTURE_2D, m_propagation2DVolumeBlue1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, static_cast<GLsizei>(VOLUME_SIZE * VOLUME_SIZE), static_cast<GLsizei>(VOLUME_SIZE), 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, m_propagation2DVolumeBlue1, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void LightPropagationVolumes::render(const RenderData &_renderData, const Scene &_scene, const std::shared_ptr<Level> &_level, RenderPass **_previousRenderPass)
{
	glm::ivec3 volumeDimensions = glm::ivec3(static_cast<int>(VOLUME_SIZE));
	float volumeSpacing = 0.5f;
	glm::vec2 target = glm::vec2(0.0f);
	glm::vec3 volumeOrigin = glm::round(glm::vec3(target.x - volumeDimensions.x * volumeSpacing * 0.5f, 0.0f, target.y - volumeDimensions.z * volumeSpacing * 0.5f));
	
	// calculate during the first frame and after that each Nth frame
	if (_renderData.m_frame > 1 && _renderData.m_frame % 14)
	{
		return;
	}

	m_propagationVolume = { volumeOrigin, volumeDimensions, volumeSpacing };

	// clear the volume
	glm::vec4 d(0.0);
	glClearTexImage(m_propagation2DAccumVolumeRed, 0, GL_RGBA, GL_FLOAT, &d);
	glClearTexImage(m_propagation2DAccumVolumeGreen, 0, GL_RGBA, GL_FLOAT, &d);
	glClearTexImage(m_propagation2DAccumVolumeBlue, 0, GL_RGBA, GL_FLOAT, &d);

	glBindImageTexture(3, m_propagation2DAccumVolumeRed, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(4, m_propagation2DAccumVolumeGreen, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(5, m_propagation2DAccumVolumeBlue, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	auto light = _level->m_lights.m_directionalLights[0];
	glm::vec3 lightDir = light->getDirection();
	glm::vec3 upDir(0.0f, 1.0f, 0.0f);
	// choose different up vector if light direction would be linearly dependent otherwise
	if (abs(lightDir.x) < 0.001 && abs(lightDir.z) < 0.001)
	{
		upDir = glm::vec3(1.0f, 1.0f, 0.0f);
	}

	glm::vec3 volumeWorldSpaceCenter = volumeOrigin + glm::vec3(volumeDimensions) * volumeSpacing * 0.5f;

	glm::vec3 lightPos = lightDir * 100.0f + volumeWorldSpaceCenter;

	glm::mat4 lightView = glm::lookAt(lightPos, volumeWorldSpaceCenter, upDir);
	float projectionHalfWidth = VOLUME_SIZE * volumeSpacing * 0.5f;
	glm::mat4 projection = glm::ortho(-projectionHalfWidth, projectionHalfWidth, -projectionHalfWidth, projectionHalfWidth, 0.0f, 500.0f);
	glm::mat4 viewProjection = projection * lightView;

	m_rsmRenderPass->render(viewProjection, light, _scene, _previousRenderPass);

	glm::mat4 invViewProjection = glm::inverse(viewProjection);

	m_lightInjectionRenderPass->render(m_propagationVolume, invViewProjection, 0, m_rsmFlux, m_rsmNormal, _previousRenderPass);
	Volume geometryVolume = m_propagationVolume;
	geometryVolume.m_origin += 0.5f * volumeSpacing;
	m_geometryInjectionRenderPass->render(geometryVolume, invViewProjection, m_rsmNormal, lightDir, _previousRenderPass);

	GLuint redTextures[] = { m_propagation2DVolumeRed0, m_propagation2DVolumeRed1 };
	GLuint greenTextures[] = { m_propagation2DVolumeGreen0, m_propagation2DVolumeGreen1 };
	GLuint blueTextures[] = { m_propagation2DVolumeBlue0, m_propagation2DVolumeBlue1 };

	m_lightPropagationRenderPass->render(m_propagationVolume, m_geometry2DVolume, redTextures, greenTextures, blueTextures, _previousRenderPass);
}

GLuint LightPropagationVolumes::getRedVolume() const
{
	return m_propagation2DAccumVolumeRed;
}

GLuint LightPropagationVolumes::getGreenVolume() const
{
	return m_propagation2DAccumVolumeGreen;
}

GLuint LightPropagationVolumes::getBlueVolume() const
{
	return m_propagation2DAccumVolumeBlue;
}

GLuint LightPropagationVolumes::get2DVolume() const
{
	return m_propagation2DAccumVolumeRed;
}

Volume LightPropagationVolumes::getVolume() const
{
	return m_propagationVolume;
}
