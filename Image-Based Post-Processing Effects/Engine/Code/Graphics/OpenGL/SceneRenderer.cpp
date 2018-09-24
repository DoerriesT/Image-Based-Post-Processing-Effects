#define GLFW_INCLUDE_NONE
#include <GLFW\glfw3.h>
#include <iostream>
#include <glm\mat4x4.hpp>
#include <glm\vec3.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\ext.hpp>
#include <random>
#include <memory>
#include <limits.h>
#include "SceneRenderer.h"
#include "Utilities\ContainerUtility.h"
#include "Engine.h"
#include "EntityComponentSystem\EntityManager.h"
#include "ShaderProgram.h"
#include "Graphics\Camera.h"
#include "Window\Window.h"
#include "Level.h"
#include "Graphics\Mesh.h"
#include "Graphics\EntityRenderData.h"
#include "RenderData.h"
#include "Graphics\Scene.h"
#include "Graphics\Effects.h"
#include "Graphics\Texture.h"
#include "Input\UserInput.h"
#include "RenderPass\Shadow\ShadowRenderPass.h"
#include "RenderPass\Geometry\GBufferRenderPass.h"
#include "RenderPass\Geometry\GBufferCustomRenderPass.h"
#include "RenderPass\SSAO\SSAOOriginalRenderPass.h"
#include "RenderPass\SSAO\SSAORenderPass.h"
#include "RenderPass\SSAO\HBAORenderPass.h"
#include "RenderPass\SSAO\GTAORenderPass.h"
#include "RenderPass\SSAO\GTAOSpatialDenoiseRenderPass.h"
#include "RenderPass\SSAO\GTAOTemporalDenoiseRenderPass.h"
#include "RenderPass\SSAO\SSAOBlurRenderPass.h"
#include "RenderPass\SSAO\SSAOBilateralBlurRenderPass.h"
#include "RenderPass\Geometry\SkyboxRenderPass.h"
#include "RenderPass\Lighting\AmbientLightRenderPass.h"
#include "RenderPass\Lighting\DirectionalLightRenderPass.h"
#include "RenderPass\Lighting\StencilRenderPass.h"
#include "RenderPass\Lighting\DeferredEnvironmentProbeRenderPass.h"
#include "RenderPass\Lighting\PointLightRenderPass.h"
#include "RenderPass\Lighting\SpotLightRenderPass.h"
#include "RenderPass\Geometry\ForwardRenderPass.h"
#include "RenderPass\Geometry\ForwardCustomRenderPass.h"
#include "RenderPass\Geometry\OutlineRenderPass.h"
#include "RenderPass\Geometry\LightProbeRenderPass.h"

SceneRenderer::SceneRenderer(std::shared_ptr<Window> _window)
	:window(_window), ocean(false, true), volumetricLighting(window->getWidth(), window->getHeight())
{
}

SceneRenderer::~SceneRenderer()
{
	GLuint textures[] = { gAlbedoTexture, gNormalTexture, gMRASTexture, gDepthStencilTexture, gLightColorTextures[0], gLightColorTextures[1],
		gVelocityTexture, brdfLUT };
	glDeleteTextures(sizeof(textures) / sizeof(GLuint), textures);

	GLuint fbos[] = { gBufferFBO, ssaoFbo, shadowFbo };

	glDeleteFramebuffers(sizeof(fbos) / sizeof(GLuint), fbos);
}

void SceneRenderer::init()
{
	// create FBO
	glGenFramebuffers(1, &gBufferFBO);
	glGenFramebuffers(1, &ssaoFbo);
	glGenFramebuffers(1, &shadowFbo);

	auto res = std::make_pair(window->getWidth(), window->getHeight());
	createFboAttachments(res);
	createSsaoAttachments(res);

	gbuffer.albedoTexture = gAlbedoTexture;
	gbuffer.normalTexture = gNormalTexture;
	gbuffer.materialTexture = gMRASTexture;
	gbuffer.lightTextures[0] = gLightColorTextures[0];
	gbuffer.lightTextures[1] = gLightColorTextures[1];
	gbuffer.velocityTexture = gVelocityTexture;
	gbuffer.ssaoTexture = ssaoTextureA;
	gbuffer.depthStencilTexture = gDepthStencilTexture;

	createBrdfLUT();

	ocean.init(gBufferFBO, res.first, res.second);
	volumetricLighting.init();
	lightPropagationVolumes.init();

	shadowRenderPass = new ShadowRenderPass(shadowFbo, 1, 1); // viewport is reconfigured for every light so the constructor value does not matter
	gBufferRenderPass = new GBufferRenderPass(gBufferFBO, res.first, res.second);
	gBufferCustomRenderPass = new GBufferCustomRenderPass(gBufferFBO, res.first, res.second);
	ssaoOriginalRenderPass = new SSAOOriginalRenderPass(ssaoFbo, res.first, res.second);
	ssaoRenderPass = new SSAORenderPass(ssaoFbo, res.first, res.second);
	hbaoRenderPass = new HBAORenderPass(ssaoFbo, res.first, res.second);
	gtaoRenderPass = new GTAORenderPass(ssaoFbo, res.first, res.second);
	gtaoSpatialDenoiseRenderPass = new GTAOSpatialDenoiseRenderPass(ssaoFbo, res.first, res.second);
	gtaoTemporalDenoiseRenderPass = new GTAOTemporalDenoiseRenderPass(ssaoFbo, res.first, res.second);
	ssaoBlurRenderPass = new SSAOBlurRenderPass(ssaoFbo, res.first, res.second);
	ssaoBilateralBlurRenderPass = new SSAOBilateralBlurRenderPass(ssaoFbo, res.first, res.second);
	skyboxRenderPass = new SkyboxRenderPass(gBufferFBO, res.first, res.second);
	ambientLightRenderPass = new AmbientLightRenderPass(gBufferFBO, res.first, res.second);
	directionalLightRenderPass = new DirectionalLightRenderPass(gBufferFBO, res.first, res.second);
	stencilRenderPass = new StencilRenderPass(gBufferFBO, res.first, res.second);
	deferredEnvironmentProbeRenderPass = new DeferredEnvironmentProbeRenderPass(gBufferFBO, res.first, res.second);
	pointLightRenderPass = new PointLightRenderPass(gBufferFBO, res.first, res.second);
	spotLightRenderPass = new SpotLightRenderPass(gBufferFBO, res.first, res.second);
	forwardRenderPass = new ForwardRenderPass(gBufferFBO, res.first, res.second);
	forwardCustomRenderPass = new ForwardCustomRenderPass(gBufferFBO, res.first, res.second);
	outlineRenderPass = new OutlineRenderPass(gBufferFBO, res.first, res.second);
	lightProbeRenderPass = new LightProbeRenderPass(gBufferFBO, res.first, res.second);
}

void SceneRenderer::render(const RenderData &_renderData, const Scene &_scene, const std::shared_ptr<Level> &_level, const Effects &_effects)
{
	RenderPass *previousRenderPass = nullptr;
	frame = _renderData.frame;

	if (_effects.diffuseAmbientSource == DiffuseAmbientSource::LIGHT_PROPAGATION_VOLUMES)
	{
		lightPropagationVolumes.render(_renderData, _scene, _level, &previousRenderPass);
	}
	
	shadowRenderPass->render(_renderData, _level, _scene, true, &previousRenderPass);

	if (_level->water.enabled)
	{
		ocean.prepareRender(_renderData, _level, &previousRenderPass);
	}

	const GLenum firstPassDrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 , lightColorAttachments[frame % 2], GL_COLOR_ATTACHMENT6 };

	// bind g-buffer
	glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
	glDrawBuffers(sizeof(firstPassDrawBuffers) / sizeof(GLenum), firstPassDrawBuffers);

	// enable depth testing and writing and clear all buffers
	glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	gBufferRenderPass->render(_renderData, _scene, &previousRenderPass);
	gBufferCustomRenderPass->render(_renderData, _level, _scene, &previousRenderPass);

	// setup all g-buffer textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gAlbedoTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormalTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gMRASTexture);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gDepthStencilTexture);

	switch (_effects.ambientOcclusion)
	{
	case AmbientOcclusion::SSAO_ORIGINAL:
	{
		ssaoOriginalRenderPass->render(_renderData, _effects, gbuffer, noiseTexture, &previousRenderPass);
		ssaoBilateralBlurRenderPass->render(_renderData, _effects, gbuffer, ssaoTextureA, &previousRenderPass);
		gbuffer.ssaoTexture = ssaoTextureB;
		break;
	}
	case AmbientOcclusion::SSAO:
	{
		ssaoRenderPass->render(_renderData, _effects, gbuffer, noiseTexture, &previousRenderPass);
		ssaoBilateralBlurRenderPass->render(_renderData, _effects, gbuffer, ssaoTextureA, &previousRenderPass);
		gbuffer.ssaoTexture = ssaoTextureB;
		break;
	}
	case AmbientOcclusion::HBAO:
	{
		hbaoRenderPass->render(_renderData, _effects, gbuffer, noiseTexture2, &previousRenderPass);
		ssaoBilateralBlurRenderPass->render(_renderData, _effects, gbuffer, ssaoTextureA, &previousRenderPass);
		gbuffer.ssaoTexture = ssaoTextureB;
		break;
	}
	case AmbientOcclusion::GTAO:
	{
		gtaoRenderPass->render(_renderData, _effects, gbuffer, &previousRenderPass);
		GLuint ssaoTextures[3] = { ssaoTextureA, ssaoTextureB, ssaoTextureC };
		gtaoSpatialDenoiseRenderPass->render(_renderData, _effects, gbuffer, ssaoTextures, &previousRenderPass);
		gtaoTemporalDenoiseRenderPass->render(_renderData, _effects, gbuffer, ssaoTextures, &previousRenderPass);
		gbuffer.ssaoTexture = _renderData.frame % 2 ? ssaoTextures[2] : ssaoTextures[0];
		break;
	}
	default:
		break;
	}

	skyboxRenderPass->render(_renderData, _level, &previousRenderPass);
	GLuint volumes[] = { lightPropagationVolumes.getRedVolume(), lightPropagationVolumes.getGreenVolume(), lightPropagationVolumes.getBlueVolume() };
	ambientLightRenderPass->render(_renderData, _level, _effects, gbuffer, brdfLUT, volumes, lightPropagationVolumes.getVolume(), &previousRenderPass);
	directionalLightRenderPass->render(_renderData, _level, gbuffer, &previousRenderPass);
	stencilRenderPass->render(_renderData, _level, gbuffer, &previousRenderPass);
	deferredEnvironmentProbeRenderPass->render(_renderData, _level, _effects, gbuffer, brdfLUT, &previousRenderPass);
	pointLightRenderPass->render(_renderData, _level, gbuffer, &previousRenderPass);
	spotLightRenderPass->render(_renderData, _level, gbuffer, &previousRenderPass);
	forwardRenderPass->render(_renderData, _level, _scene, &previousRenderPass);
	forwardCustomRenderPass->render(_renderData, _level, _scene, &previousRenderPass);
	outlineRenderPass->render(_renderData, _scene, &previousRenderPass);
	lightProbeRenderPass->render(_renderData, _level, &previousRenderPass);

	if (_level->water.enabled)
	{
		ocean.render(_renderData, _level, &previousRenderPass);
	}

	// generate mips
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gLightColorTextures[frame % 2]);
	glGenerateMipmap(GL_TEXTURE_2D);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

void SceneRenderer::resize(const std::pair<unsigned int, unsigned int> &_resolution)
{
	GLuint textures[] = 
	{ 
		gAlbedoTexture, 
		gNormalTexture,
		gMRASTexture,
		gLightColorTextures[0],
		gLightColorTextures[1],
		gVelocityTexture,
		gDepthStencilTexture,    
		ssaoTextureA, 
		ssaoTextureB, 
		ssaoTextureC,
		noiseTexture,
		noiseTexture2
	};

	glDeleteTextures(sizeof(textures) / sizeof(GLuint), textures);
	createFboAttachments(_resolution);
	createSsaoAttachments(_resolution);
	volumetricLighting.resize(_resolution.first, _resolution.second);

	gBufferRenderPass->resize(_resolution.first, _resolution.second);
	gBufferCustomRenderPass->resize(_resolution.first, _resolution.second);
	ssaoOriginalRenderPass->resize(_resolution.first, _resolution.second);
	ssaoRenderPass->resize(_resolution.first, _resolution.second);
	hbaoRenderPass->resize(_resolution.first, _resolution.second);
	gtaoRenderPass->resize(_resolution.first, _resolution.second);
	gtaoSpatialDenoiseRenderPass->resize(_resolution.first, _resolution.second);
	gtaoTemporalDenoiseRenderPass->resize(_resolution.first, _resolution.second);
	ssaoBlurRenderPass->resize(_resolution.first, _resolution.second);
	ssaoBilateralBlurRenderPass->resize(_resolution.first, _resolution.second);
	skyboxRenderPass->resize(_resolution.first, _resolution.second);
	ambientLightRenderPass->resize(_resolution.first, _resolution.second);
	directionalLightRenderPass->resize(_resolution.first, _resolution.second);
	stencilRenderPass->resize(_resolution.first, _resolution.second);
	deferredEnvironmentProbeRenderPass->resize(_resolution.first, _resolution.second);
	pointLightRenderPass->resize(_resolution.first, _resolution.second);
	spotLightRenderPass->resize(_resolution.first, _resolution.second);
	forwardRenderPass->resize(_resolution.first, _resolution.second);
	forwardCustomRenderPass->resize(_resolution.first, _resolution.second);
	outlineRenderPass->resize(_resolution.first, _resolution.second);
	lightProbeRenderPass->resize(_resolution.first, _resolution.second);

	gbuffer.albedoTexture = gAlbedoTexture;
	gbuffer.normalTexture = gNormalTexture;
	gbuffer.materialTexture = gMRASTexture;
	gbuffer.lightTextures[0] = gLightColorTextures[0];
	gbuffer.lightTextures[1] = gLightColorTextures[1];
	gbuffer.velocityTexture = gVelocityTexture;
	gbuffer.ssaoTexture = frame % 2 ? ssaoTextureC : ssaoTextureA;
	gbuffer.depthStencilTexture = gDepthStencilTexture;
}

GLuint SceneRenderer::getColorTexture() const
{
	return gLightColorTextures[frame % 2];
}

GLuint SceneRenderer::getAlbedoTexture() const
{
	return gAlbedoTexture;
}

GLuint SceneRenderer::getNormalTexture() const
{
	return gNormalTexture;
}

GLuint SceneRenderer::getMaterialTexture() const
{
	return gMRASTexture;
}

GLuint SceneRenderer::getDepthStencilTexture() const
{
	return gDepthStencilTexture;
}

GLuint SceneRenderer::getVelocityTexture() const
{
	return gVelocityTexture;
}

GLuint SceneRenderer::getAmbientOcclusionTexture() const
{
	return gbuffer.ssaoTexture;
}

GLuint SceneRenderer::getBrdfLUT() const
{
	return brdfLUT;
}

void SceneRenderer::createFboAttachments(const std::pair<unsigned int, unsigned int> &_resolution)
{
	glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);

	glGenTextures(1, &gAlbedoTexture);
	glBindTexture(GL_TEXTURE_2D, gAlbedoTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, _resolution.first, _resolution.second, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gAlbedoTexture, 0);

	glGenTextures(1, &gNormalTexture);
	glBindTexture(GL_TEXTURE_2D, gNormalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, _resolution.first, _resolution.second, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormalTexture, 0);

	glGenTextures(1, &gMRASTexture);
	glBindTexture(GL_TEXTURE_2D, gMRASTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _resolution.first, _resolution.second, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gMRASTexture, 0);

	glGenTextures(1, &gVelocityTexture);
	glBindTexture(GL_TEXTURE_2D, gVelocityTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _resolution.first, _resolution.second, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gVelocityTexture, 0);

	glGenTextures(1, &gLightColorTextures[0]);
	glBindTexture(GL_TEXTURE_2D, gLightColorTextures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, _resolution.first, _resolution.second, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, gLightColorTextures[0], 0);

	glGenTextures(1, &gLightColorTextures[1]);
	glBindTexture(GL_TEXTURE_2D, gLightColorTextures[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, _resolution.first, _resolution.second, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, gLightColorTextures[1], 0);

	lightColorAttachments[0] = GL_COLOR_ATTACHMENT4;
	lightColorAttachments[1] = GL_COLOR_ATTACHMENT5;

	glGenTextures(1, &gDepthStencilTexture);
	glBindTexture(GL_TEXTURE_2D, gDepthStencilTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, _resolution.first, _resolution.second, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, gDepthStencilTexture, 0);

	GLenum gBufferAttachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(sizeof(gBufferAttachments) / sizeof(GLenum), gBufferAttachments);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "G-Buffer FBO not complete! " << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneRenderer::createSsaoAttachments(const std::pair<unsigned int, unsigned int> &_resolution)
{
	std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
	std::default_random_engine generator;
	glm::vec3 ssaoNoise[16];
	for (unsigned int i = 0; i < 16; ++i)
	{
		glm::vec3 noise(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0);
		ssaoNoise[i] = noise;
	}

	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, ssaoNoise);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	std::uniform_real_distribution<GLfloat> hbaoAlphaDist(0.0f, 2.0f * glm::pi<float>() / 4.0f);
	std::uniform_real_distribution<GLfloat> hbaoBetaDist(0.0f, 0.999999f);
	glm::vec3 hbaoNoise[16];
	for (unsigned int i = 0; i < 16; ++i)
	{
		float alpha = hbaoAlphaDist(generator);
		float beta = hbaoBetaDist(generator);
		glm::vec3 noise(
			glm::cos(alpha),
			glm::sin(alpha),
			beta);
		hbaoNoise[i] = noise;
	}

	glGenTextures(1, &noiseTexture2);
	glBindTexture(GL_TEXTURE_2D, noiseTexture2);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, hbaoNoise);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFbo);

	glGenTextures(1, &ssaoTextureA);
	glBindTexture(GL_TEXTURE_2D, ssaoTextureA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _resolution.first, _resolution.second, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoTextureA, 0);

	glGenTextures(1, &ssaoTextureB);
	glBindTexture(GL_TEXTURE_2D, ssaoTextureB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _resolution.first, _resolution.second, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, ssaoTextureB, 0);

	glGenTextures(1, &ssaoTextureC);
	glBindTexture(GL_TEXTURE_2D, ssaoTextureC);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _resolution.first, _resolution.second, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, ssaoTextureC, 0);
}

void SceneRenderer::createBrdfLUT()
{
	std::shared_ptr<ShaderProgram> brdfShader = ShaderProgram::createShaderProgram("Resources/Shaders/Lighting/brdf.comp");

	glGenTextures(1, &brdfLUT);

	glBindTexture(GL_TEXTURE_2D, brdfLUT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_HALF_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	brdfShader->bind();
	glBindImageTexture(0, brdfLUT, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
	glDispatchCompute(512 / 8, 512 / 8, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}