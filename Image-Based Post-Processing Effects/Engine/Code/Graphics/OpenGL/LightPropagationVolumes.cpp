#include "LightPropagationVolumes.h"
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\RenderPass\LPV\RSMRenderPass.h"
#include "Graphics\OpenGL\RenderPass\LPV\LightInjectionRenderPass.h"
#include "Graphics\OpenGL\RenderPass\LPV\LightPropagationRenderPass.h"
#include "Graphics\OpenGL\RenderData.h"
#include "Level.h"

int RSM_SIZE = 256;
int VOLUME_SIZE = 32;

void LightPropagationVolumes::init()
{
	glCreateFramebuffers(1, &propagationFbo);
	glCreateFramebuffers(1, &rsmFbo);

	rsmRenderPass = std::make_unique<RSMRenderPass>(rsmFbo, RSM_SIZE, RSM_SIZE);
	lightInjectionRenderPass = std::make_unique<LightInjectionRenderPass>(propagationFbo, RSM_SIZE / 4, RSM_SIZE / 4);
	lightPropagationRenderPass = std::make_unique<LightPropagationRenderPass>(propagationFbo, VOLUME_SIZE * VOLUME_SIZE, VOLUME_SIZE);

	// RSM textures
	{
		glBindFramebuffer(GL_FRAMEBUFFER, rsmFbo);

		glGenTextures(1, &rsmDepth);
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

		glGenTextures(1, &propagation2DVolumeRed0);
		glBindTexture(GL_TEXTURE_2D, propagation2DVolumeRed0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, VOLUME_SIZE * VOLUME_SIZE, VOLUME_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, propagation2DVolumeRed0, 0);

		glGenTextures(1, &propagation2DVolumeGreen0);
		glBindTexture(GL_TEXTURE_2D, propagation2DVolumeGreen0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, VOLUME_SIZE * VOLUME_SIZE, VOLUME_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, propagation2DVolumeGreen0, 0);

		glGenTextures(1, &propagation2DVolumeBlue0);
		glBindTexture(GL_TEXTURE_2D, propagation2DVolumeBlue0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, VOLUME_SIZE * VOLUME_SIZE, VOLUME_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, propagation2DVolumeBlue0, 0);

		glGenTextures(1, &propagation2DVolumeRed1);
		glBindTexture(GL_TEXTURE_2D, propagation2DVolumeRed1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, VOLUME_SIZE * VOLUME_SIZE, VOLUME_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, propagation2DVolumeRed1, 0);

		glGenTextures(1, &propagation2DVolumeGreen1);
		glBindTexture(GL_TEXTURE_2D, propagation2DVolumeGreen1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, VOLUME_SIZE * VOLUME_SIZE, VOLUME_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, propagation2DVolumeGreen1, 0);

		glGenTextures(1, &propagation2DVolumeBlue1);
		glBindTexture(GL_TEXTURE_2D, propagation2DVolumeBlue1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, VOLUME_SIZE * VOLUME_SIZE, VOLUME_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, propagation2DVolumeBlue1, 0);
	}
}

void LightPropagationVolumes::render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, RenderPass **_previousRenderPass)
{
}
