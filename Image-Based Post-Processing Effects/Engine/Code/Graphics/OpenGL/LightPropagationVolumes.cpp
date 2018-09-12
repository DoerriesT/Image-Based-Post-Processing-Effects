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

int RSM_SIZE = 512;
int VOLUME_SIZE = 32;

void LightPropagationVolumes::init()
{
	glCreateFramebuffers(1, &propagationFbo);
	glCreateFramebuffers(1, &rsmFbo);

	rsmRenderPass = std::make_unique<RSMRenderPass>(rsmFbo, RSM_SIZE, RSM_SIZE);
	lightInjectionRenderPass = std::make_unique<LightInjectionRenderPass>(propagationFbo, VOLUME_SIZE * VOLUME_SIZE, VOLUME_SIZE);
	geometryInjectionRenderPass = std::make_unique<GeometryInjectionRenderPass>(propagationFbo, VOLUME_SIZE * VOLUME_SIZE, VOLUME_SIZE);
	lightPropagationRenderPass = std::make_unique<LightPropagationRenderPass>(propagationFbo, VOLUME_SIZE * VOLUME_SIZE, VOLUME_SIZE);

	// RSM textures
	{
		glBindFramebuffer(GL_FRAMEBUFFER, rsmFbo);

		glGenTextures(1, &rsmDepth);
		glBindTexture(GL_TEXTURE_2D, rsmDepth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, RSM_SIZE, RSM_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, rsmDepth, 0);

		glGenTextures(1, &rsmFlux);
		glBindTexture(GL_TEXTURE_2D, rsmFlux);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, RSM_SIZE, RSM_SIZE, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rsmFlux, 0);

		glGenTextures(1, &rsmNormal);
		glBindTexture(GL_TEXTURE_2D, rsmNormal);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, RSM_SIZE, RSM_SIZE, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, rsmNormal, 0);
	}

	// 3d textures
	{
		glGenTextures(1, &propagationVolumeRed);
		glBindTexture(GL_TEXTURE_3D, propagationVolumeRed);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, VOLUME_SIZE, VOLUME_SIZE, VOLUME_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);

		glGenTextures(1, &propagationVolumeGreen);
		glBindTexture(GL_TEXTURE_3D, propagationVolumeGreen);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, VOLUME_SIZE, VOLUME_SIZE, VOLUME_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);

		glGenTextures(1, &propagationVolumeBlue);
		glBindTexture(GL_TEXTURE_3D, propagationVolumeBlue);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, VOLUME_SIZE, VOLUME_SIZE, VOLUME_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);
	}

	// 2d textures
	{
		glBindFramebuffer(GL_FRAMEBUFFER, propagationFbo);

		glGenTextures(1, &geometry2DVolume);
		glBindTexture(GL_TEXTURE_2D, geometry2DVolume);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, VOLUME_SIZE * VOLUME_SIZE, VOLUME_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, geometry2DVolume, 0);

		glGenTextures(1, &propagation2DAccumVolumeRed);
		glBindTexture(GL_TEXTURE_2D, propagation2DAccumVolumeRed);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, VOLUME_SIZE * VOLUME_SIZE, VOLUME_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenTextures(1, &propagation2DAccumVolumeGreen);
		glBindTexture(GL_TEXTURE_2D, propagation2DAccumVolumeGreen);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, VOLUME_SIZE * VOLUME_SIZE, VOLUME_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenTextures(1, &propagation2DAccumVolumeBlue);
		glBindTexture(GL_TEXTURE_2D, propagation2DAccumVolumeBlue);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, VOLUME_SIZE * VOLUME_SIZE, VOLUME_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenTextures(1, &propagation2DVolumeRed0);
		glBindTexture(GL_TEXTURE_2D, propagation2DVolumeRed0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, VOLUME_SIZE * VOLUME_SIZE, VOLUME_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, propagation2DVolumeRed0, 0);

		glGenTextures(1, &propagation2DVolumeGreen0);
		glBindTexture(GL_TEXTURE_2D, propagation2DVolumeGreen0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, VOLUME_SIZE * VOLUME_SIZE, VOLUME_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, propagation2DVolumeGreen0, 0);

		glGenTextures(1, &propagation2DVolumeBlue0);
		glBindTexture(GL_TEXTURE_2D, propagation2DVolumeBlue0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, VOLUME_SIZE * VOLUME_SIZE, VOLUME_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, propagation2DVolumeBlue0, 0);

		glGenTextures(1, &propagation2DVolumeRed1);
		glBindTexture(GL_TEXTURE_2D, propagation2DVolumeRed1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, VOLUME_SIZE * VOLUME_SIZE, VOLUME_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, propagation2DVolumeRed1, 0);

		glGenTextures(1, &propagation2DVolumeGreen1);
		glBindTexture(GL_TEXTURE_2D, propagation2DVolumeGreen1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, VOLUME_SIZE * VOLUME_SIZE, VOLUME_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, propagation2DVolumeGreen1, 0);

		glGenTextures(1, &propagation2DVolumeBlue1);
		glBindTexture(GL_TEXTURE_2D, propagation2DVolumeBlue1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, VOLUME_SIZE * VOLUME_SIZE, VOLUME_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, propagation2DVolumeBlue1, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void LightPropagationVolumes::render(const RenderData &_renderData, const Scene &_scene, const std::shared_ptr<Level> &_level, RenderPass **_previousRenderPass)
{
	glm::ivec3 volumeDimensions = glm::ivec3(VOLUME_SIZE);
	float volumeSpacing = 1.0f;
	glm::vec2 target = glm::vec2(_renderData.cameraPosition.x, _renderData.cameraPosition.z) + 8.0f * glm::vec2(_renderData.viewDirection.x, _renderData.viewDirection.z);
	glm::vec3 volumeOrigin = glm::round(glm::vec3(target.x - volumeDimensions.x * volumeSpacing * 0.5f, 0.0f, target.y - volumeDimensions.z * volumeSpacing * 0.5f));
	
	// calculate during the first frame and after that each Nth frame
	if (_renderData.frame > 1 && _renderData.frame % 14)
	{
		return;
	}

	propagationVolume = { volumeOrigin, volumeDimensions, volumeSpacing };

	// clear the volume
	glm::vec4 d(0.0);
	glClearTexImage(propagation2DAccumVolumeRed, 0, GL_RGBA, GL_FLOAT, &d);
	glClearTexImage(propagation2DAccumVolumeGreen, 0, GL_RGBA, GL_FLOAT, &d);
	glClearTexImage(propagation2DAccumVolumeBlue, 0, GL_RGBA, GL_FLOAT, &d);

	glBindImageTexture(3, propagation2DAccumVolumeRed, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(4, propagation2DAccumVolumeGreen, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(5, propagation2DAccumVolumeBlue, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	auto light = _level->lights.directionalLights[0];
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
	glm::mat4 projection = glm::ortho(-16.0f, 16.0f, -16.0f, 16.0f, 0.0f, 500.0f);
	glm::mat4 viewProjection = projection * lightView;

	rsmRenderPass->render(viewProjection, light, _scene, _previousRenderPass);

	glm::mat4 invViewProjection = glm::inverse(viewProjection);

	lightInjectionRenderPass->render(propagationVolume, invViewProjection, 0, rsmFlux, rsmNormal, _previousRenderPass);
	Volume geometryVolume = propagationVolume;
	geometryVolume.origin += 0.5f;
	geometryInjectionRenderPass->render(geometryVolume, invViewProjection, rsmNormal, lightDir, _previousRenderPass);

	GLuint redTextures[] = { propagation2DVolumeRed0, propagation2DVolumeRed1 };
	GLuint greenTextures[] = { propagation2DVolumeGreen0, propagation2DVolumeGreen1 };
	GLuint blueTextures[] = { propagation2DVolumeBlue0, propagation2DVolumeBlue1 };
	GLuint accumTextures[] = { propagationVolumeRed, propagationVolumeGreen, propagationVolumeBlue };
	//glMemoryBarrier(GL_ALL_BARRIER_BITS);
	lightPropagationRenderPass->render(propagationVolume, geometry2DVolume, redTextures, greenTextures, blueTextures, accumTextures, _previousRenderPass);
}

GLuint LightPropagationVolumes::getRedVolume() const
{
	return propagation2DAccumVolumeRed;
}

GLuint LightPropagationVolumes::getGreenVolume() const
{
	return propagation2DAccumVolumeGreen;
}

GLuint LightPropagationVolumes::getBlueVolume() const
{
	return propagation2DAccumVolumeBlue;
}

GLuint LightPropagationVolumes::get2DVolume() const
{
	return propagation2DAccumVolumeRed;
}

Volume LightPropagationVolumes::getVolume() const
{
	return propagationVolume;
}
