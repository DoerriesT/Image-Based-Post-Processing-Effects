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


SceneRenderer::SceneRenderer(std::shared_ptr<Window> _window)
	:window(_window), ocean(false, true), volumetricLighting(window->getWidth(), window->getHeight())
{
}

SceneRenderer::~SceneRenderer()
{
	GLuint textures[] = { gAlbedoTexture, gNormalTexture, gMRASTexture, gDepthStencilTexture, gLightColorTextures[0], gLightColorTextures[1],
		gVelocityTexture, brdfLUT };
	glDeleteTextures(sizeof(textures) / sizeof(GLuint), textures);

	GLuint fbos[] = { gBufferFBO, ssaoFbo };

	glDeleteFramebuffers(sizeof(fbos) / sizeof(GLuint), fbos);
}

void SceneRenderer::init()
{
	// create shaders
	outlineShader = ShaderProgram::createShaderProgram("Resources/Shaders/Renderer/outline.vert", "Resources/Shaders/Renderer/outline.frag");
	gBufferPassShader = ShaderProgram::createShaderProgram("Resources/Shaders/Renderer/gBufferPass.vert", "Resources/Shaders/Renderer/gBufferPass.frag");
	environmentLightPassShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Renderer/globalLightPass.frag");
	pointLightPassShader = ShaderProgram::createShaderProgram("Resources/Shaders/Renderer/pointLightPass.vert", "Resources/Shaders/Renderer/pointLightPass.frag");
	spotLightPassShader = ShaderProgram::createShaderProgram("Resources/Shaders/Renderer/spotLightPass.vert", "Resources/Shaders/Renderer/spotLightPass.frag");
	directionalLightShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Renderer/directionalLightPass.frag");
	skyboxShader = ShaderProgram::createShaderProgram("Resources/Shaders/Renderer/skybox.vert", "Resources/Shaders/Renderer/skybox.frag");
	transparencyShader = ShaderProgram::createShaderProgram("Resources/Shaders/Renderer/transparencyForward.vert", "Resources/Shaders/Renderer/transparencyForward.frag");
	ssaoShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Renderer/ssao.frag");
	ssaoOriginalShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Renderer/ssaoOriginal.frag");
	ssaoBlurShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Renderer/ssaoBlur.frag");
	ssaoBilateralBlurShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Renderer/ssaoBilateralBlur.frag");
	hbaoShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Renderer/hbao.frag");

	// create uniforms

	// gBufferPass uniforms
	uMaterialG.create(gBufferPassShader);
	uModelViewProjectionMatrixG.create(gBufferPassShader);
	uPrevTransformG.create(gBufferPassShader);
	uAtlasDataG.create(gBufferPassShader);
	uVelG.create(gBufferPassShader);
	uExposureTimeG.create(gBufferPassShader);
	uMaxVelocityMagG.create(gBufferPassShader);
	uCurrTransformG.create(gBufferPassShader);
	uViewMatrixG.create(gBufferPassShader);
	uModelMatrixG.create(gBufferPassShader);
	uCamPosG.create(gBufferPassShader);

	// outline uniforms
	uModelViewProjectionMatrixO.create(outlineShader);
	uOutlineColorO.create(outlineShader);

	// environmentLightPass uniforms
	uInverseProjectionE.create(environmentLightPassShader);
	uInverseViewE.create(environmentLightPassShader);
	uDirectionalLightE.create(environmentLightPassShader);
	uShadowsEnabledE.create(environmentLightPassShader);
	uRenderDirectionalLightE.create(environmentLightPassShader);
	uSsaoE.create(environmentLightPassShader);
	uProjectionE.create(environmentLightPassShader);
	uReProjectionE.create(environmentLightPassShader);
	uUseSsrE.create(environmentLightPassShader);


	// pointLightPass uniforms
	uModelViewProjectionP.create(pointLightPassShader);
	uPointLightP.create(pointLightPassShader);
	uInverseProjectionP.create(pointLightPassShader);
	uInverseViewP.create(pointLightPassShader);
	uShadowsEnabledP.create(pointLightPassShader);
	uViewportSizeP.create(pointLightPassShader);

	// spotLightPass uniforms
	uModelViewProjectionS.create(spotLightPassShader);
	uSpotLightS.create(spotLightPassShader);
	uInverseViewS.create(spotLightPassShader);
	uInverseProjectionS.create(spotLightPassShader);
	uShadowsEnabledS.create(spotLightPassShader);
	uViewportSizeS.create(spotLightPassShader);

	// directionalLightPass uniforms
	uDirectionalLightD.create(directionalLightShader);
	uInverseViewD.create(directionalLightShader);
	uInverseProjectionD.create(directionalLightShader);
	uShadowsEnabledD.create(directionalLightShader);

	// skybox uniforms
	uInverseModelViewProjectionB.create(skyboxShader);
	uColorB.create(skyboxShader);
	uHasAlbedoMapB.create(skyboxShader);
	uCurrentToPrevTransformB.create(skyboxShader);

	// transparency uniforms
	uViewMatrixT.create(transparencyShader);
	uPrevTransformT.create(transparencyShader);
	uModelViewProjectionMatrixT.create(transparencyShader);
	uModelMatrixT.create(transparencyShader);
	uAtlasDataT.create(transparencyShader);
	uMaterialT.create(transparencyShader);
	uDirectionalLightT.create(transparencyShader);
	uRenderDirectionalLightT.create(transparencyShader);
	uCamPosT.create(transparencyShader);
	uShadowsEnabledT.create(transparencyShader);
	uCurrTransformT.create(transparencyShader);

	// ssao
	uViewAO.create(ssaoShader);
	uProjectionAO.create(ssaoShader);
	uInverseProjectionAO.create(ssaoShader);
	for (int i = 0; i < 64; ++i)
	{
		uSamplesAO.push_back(ssaoShader->createUniform(std::string("uSamples") + "[" + std::to_string(i) + "]"));
	}
	uKernelSizeAO.create(ssaoShader);
	uRadiusAO.create(ssaoShader);
	uBiasAO.create(ssaoShader);
	uStrengthAO.create(ssaoShader);

	// ssao blur
	uBlurSizeAOB.create(ssaoBlurShader);

	// ssao bilateral blur
	uSharpnessAOBB.create(ssaoBilateralBlurShader);
	uKernelRadiusAOBB.create(ssaoBilateralBlurShader);
	uInvResolutionDirectionAOBB.create(ssaoBilateralBlurShader);

	// hbao
	uFocalLengthHBAO.create(hbaoShader);
	uInverseProjectionHBAO.create(hbaoShader);
	uAOResHBAO.create(hbaoShader);
	uInvAOResHBAO.create(hbaoShader);
	uNoiseScaleHBAO.create(hbaoShader);
	uStrengthHBAO.create(hbaoShader);
	uRadiusHBAO.create(hbaoShader);
	uRadius2HBAO.create(hbaoShader);
	uNegInvR2HBAO.create(hbaoShader);
	uTanBiasHBAO.create(hbaoShader);
	uMaxRadiusPixelsHBAO.create(hbaoShader);
	uNumDirectionsHBAO.create(hbaoShader);
	uNumStepsHBAO.create(hbaoShader);

	// create FBO
	glGenFramebuffers(1, &gBufferFBO);
	glGenFramebuffers(1, &ssaoFbo);

	auto res = std::make_pair(window->getWidth(), window->getHeight());
	createFboAttachments(res);
	createSsaoAttachments(res);

	pointLightMesh = Mesh::createMesh("Resources/Models/pointlight.mesh", 1, true);
	spotLightMesh = Mesh::createMesh("Resources/Models/spotlight.mesh", 1, true);
	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);

	createBrdfLUT();

	ocean.init();
	volumetricLighting.init();
}

void SceneRenderer::render(const RenderData &_renderData, const Scene &_scene, const std::shared_ptr<Level> &_level, const Effects &_effects)
{
	// switch current result texture
	currentLightColorTexture = (currentLightColorTexture + 1) % 2;

	const GLenum firstPassDrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 , lightColorAttachments[currentLightColorTexture], GL_COLOR_ATTACHMENT6 };
	const GLenum secondPassDrawBuffers[] = { lightColorAttachments[currentLightColorTexture], GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT6 };
	glViewport(0, 0, _renderData.resolution.first, _renderData.resolution.second);

	// bind g-buffer
	glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
	glDrawBuffers(sizeof(firstPassDrawBuffers) / sizeof(GLenum), firstPassDrawBuffers);

	// enable depth testing and writing and clear all buffers
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// enable stencil testing for outlining technique
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	// render ordinary geometry
	renderGeometry(_renderData, _scene);

	// render custom opaque geometry
	renderCustomGeometry(_renderData, _level, _scene, true);

	// disable stencil testing for now; we will continue writing to the stencil when we do transparency
	glDisable(GL_STENCIL_TEST);

	// ocean
	if (_level->water.enabled)
	{
		ocean.prepareRender(_renderData, _level);
	}


	// bind light fbo and clear color attachment
	//glBindFramebuffer(GL_FRAMEBUFFER, lightBufferFBO);
	//glClear(GL_COLOR_BUFFER_BIT);

	// setup all g-buffer textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gAlbedoTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormalTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gMRASTexture);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gDepthStencilTexture);

	// disable writing to depth, since all geometry from now on is only a utility to draw lights
	glDepthMask(GL_FALSE);
	// disable depth test for the duration of rendering enviroment and directional lights
	glDisable(GL_DEPTH_TEST);

	// render ssao texture
	if (_effects.ambientOcclusion != AmbientOcclusion::OFF)
	{
		glDisable(GL_CULL_FACE);
		renderSsaoTexture(_renderData, _effects);
		glEnable(GL_CULL_FACE);
	}

	if (_effects.ambientOcclusion != AmbientOcclusion::OFF || _level->water.enabled)
	{
		glViewport(0, 0, _renderData.resolution.first, _renderData.resolution.second);
		glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
	}

	glDrawBuffers(sizeof(secondPassDrawBuffers) / sizeof(GLenum), secondPassDrawBuffers);

	// disable writing to depth, since all geometry from now on is only a utility to draw lights
	glDepthMask(GL_TRUE);
	// disable depth test for the duration of rendering enviroment and directional lights
	glEnable(GL_DEPTH_TEST);

	// render skybox
	if (_level->environment.skyboxEntity)
	{
		renderSkybox(_renderData, _level);
	}

	// disable writing to depth, since all geometry from now on is only a utility to draw lights
	glDepthMask(GL_FALSE);
	// disable depth test for the duration of rendering enviroment and directional lights
	glDisable(GL_DEPTH_TEST);

	// enable additive blending for the lights
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);


	// render environment light and the first directional light, if present
	if (_level->environment.skyboxEntity)
	{
		renderEnvironmentLight(_renderData, _level, _effects);
	}
	// render remaining directional lights
	if (_level->lights.directionalLights.size() > 1 || !_level->environment.skyboxEntity)
	{
		renderDirectionalLights(_renderData, _level);
	}

	//glEnable(GL_DEPTH_TEST);

	// cull front faces because otherwise entering a light sphere will make the light disappear
	glCullFace(GL_FRONT);

	// render point- and spotlights
	if (!_level->lights.pointLights.empty() || !_level->lights.spotLights.empty())
	{
		if (!_level->lights.pointLights.empty())
		{
			renderPointLights(_renderData, _level);
		}
		if (!_level->lights.spotLights.empty())
		{
			renderSpotLights(_renderData, _level);
		}
	}

	glEnable(GL_DEPTH_TEST);

	//volumetricLighting.render(gDepthStencilTexture, _renderData, _level);

	// rebind fbo since VolumetricLighting uses its own
	glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
	glDrawBuffers(sizeof(secondPassDrawBuffers) / sizeof(GLenum), secondPassDrawBuffers);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);



	glDepthMask(GL_TRUE);

	if (_level->water.enabled)
	{
		ocean.render(_renderData, _level);
	}

	// reenable normal face culling
	glCullFace(GL_BACK);

	// render transparency and outlines
	if (_scene.getTransparencyCount() || _scene.getOutlineCount() || _scene.getCustomTransparencyCount())
	{
		glEnable(GL_STENCIL_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


		renderTransparentGeometry(_renderData, _level, _scene);

		// render custom transparency geometry
		renderCustomGeometry(_renderData, _level, _scene, false);

		renderOutlines(_renderData, _scene);

		glDisable(GL_STENCIL_TEST);
	}

	glDepthMask(GL_FALSE);

	// from now on we are only doing fullscreen passes, so disable depth testing too
	glDisable(GL_DEPTH_TEST);

	// disable blending to save performance
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);

	// generate mips
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gLightColorTextures[currentLightColorTexture]);
	glGenerateMipmap(GL_TEXTURE_2D);
}

void SceneRenderer::resize(const std::pair<unsigned int, unsigned int> &_resolution)
{
	GLuint textures[] = { gAlbedoTexture, gNormalTexture, gMRASTexture, gDepthStencilTexture, gLightColorTextures[0], gLightColorTextures[1], gVelocityTexture, ssaoTextureA, ssaoTextureB };
	glDeleteTextures(sizeof(textures) / sizeof(GLuint), textures);
	createFboAttachments(_resolution);
	volumetricLighting.resize(_resolution.first, _resolution.second);
}

GLuint SceneRenderer::getColorTexture() const
{
	return gLightColorTextures[currentLightColorTexture];
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
	return ssaoTextureA;
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _resolution.first, _resolution.second, 0, GL_RG, GL_FLOAT, NULL);
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
}

void SceneRenderer::renderGeometry(const RenderData &_renderData, const Scene &_scene)
{
	gBufferPassShader->bind();

	const std::vector<std::unique_ptr<EntityRenderData>> &data = _scene.getData();

	std::shared_ptr<SubMesh> currentMesh = nullptr;
	bool enabledMesh = false;

	for (std::size_t i = 0; i < data.size(); ++i)
	{
		const std::unique_ptr<EntityRenderData> &entityRenderData = data[i];

		// skip this iteration if its supposed to be rendered with another method or does not have sufficient components
		if (entityRenderData->customOpaqueShaderComponent ||
			entityRenderData->customTransparencyShaderComponent ||
			!entityRenderData->modelComponent ||
			!entityRenderData->transformationComponent)
		{
			continue;
		}

		if (currentMesh != entityRenderData->mesh)
		{
			currentMesh = entityRenderData->mesh;
			enabledMesh = false;
		}

		// skip this mesh if its transparent
		if (entityRenderData->transparencyComponent && ContainerUtility::contains(entityRenderData->transparencyComponent->transparentSubMeshes, currentMesh))
		{
			continue;
		}

		// skip this iteration if the mesh is not yet valid
		if (!currentMesh || !currentMesh->isValid())
		{
			continue;
		}

		glm::mat4 modelMatrix = glm::translate(entityRenderData->transformationComponent->position)
			* glm::mat4_cast(entityRenderData->transformationComponent->rotation)
			* glm::scale(glm::vec3(entityRenderData->transformationComponent->scale));

		int rows = 1;
		int columns = 1;
		glm::vec2 textureOffset;
		TextureAtlasIndexComponent *textureAtlasComponent = entityRenderData->textureAtlasIndexComponent;
		if (textureAtlasComponent && ContainerUtility::contains(textureAtlasComponent->meshToIndexMap, currentMesh))
		{
			rows = textureAtlasComponent->rows;
			columns = textureAtlasComponent->columns;
			int texPos = textureAtlasComponent->meshToIndexMap[currentMesh];
			int col = texPos % columns;
			int row = texPos / columns;
			textureOffset = glm::vec2((float)col / columns, (float)row / rows);
		}

		glm::mat4 mvpTransformation = _renderData.viewProjectionMatrix * modelMatrix;
		const float cameraMovementStrength = 0.15f;
		glm::mat4 prevTransformation = glm::mix(_renderData.invJitter * _renderData.viewProjectionMatrix, _renderData.prevInvJitter * _renderData.prevViewProjectionMatrix, cameraMovementStrength) * entityRenderData->transformationComponent->prevTransformation;

		if (cullAABB(mvpTransformation, currentMesh->getAABB()))
		{
			continue;
		}

		uCamPosG.set(_renderData.cameraPosition);
		uViewMatrixG.set(glm::mat3(_renderData.viewMatrix));
		uModelMatrixG.set(modelMatrix);
		uAtlasDataG.set(glm::vec4(1.0f / columns, 1.0f / rows, textureOffset));
		uModelViewProjectionMatrixG.set(mvpTransformation);
		uPrevTransformG.set(prevTransformation);
		uCurrTransformG.set(_renderData.invJitter * mvpTransformation);
		uVelG.set(entityRenderData->transformationComponent->vel / glm::vec2(_renderData.resolution.first, _renderData.resolution.second));
		const float frameRateTarget = 60.0f;
		uExposureTimeG.set((float(Engine::getFps()) / frameRateTarget));
		const float tileSize = 40.0f;
		uMaxVelocityMagG.set(glm::length(glm::vec2(1.0f) / glm::vec2(_renderData.resolution.first, _renderData.resolution.second)) * tileSize);

		entityRenderData->transformationComponent->prevTransformation = modelMatrix;

		if (entityRenderData->outlineComponent)
		{
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
			glStencilMask(0xFF);
		}

		if (!enabledMesh)
		{
			enabledMesh = true;
			currentMesh->enableVertexAttribArrays();
		}

		// we're good to go: render this mesh-entity instance
		uMaterialG.set(entityRenderData->material);
		entityRenderData->material->bindTextures();

		currentMesh->render();

		if (entityRenderData->outlineComponent)
		{
			glStencilMask(0x00);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		}
	}
}

void SceneRenderer::renderSkybox(const RenderData &_renderData, const std::shared_ptr<Level> &_level)
{
	static const glm::vec4 DEFAULT_ALBEDO_COLOR(1.0);
	static glm::mat4 prevTransform;

	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();
	skyboxShader->bind();

	EntityManager &entityManager = EntityManager::getInstance();

	uHasAlbedoMapB.set(_level->environment.environmentMap ? true : false);
	uColorB.set(DEFAULT_ALBEDO_COLOR);

	if (_level->environment.environmentMap)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(_level->environment.environmentMap->getTarget(), _level->environment.environmentMap->getId());
	}

	TransformationComponent *transformationComponent = entityManager.getComponent<TransformationComponent>(_level->environment.skyboxEntity);
	glm::mat4 mvpMatrix = _renderData.projectionMatrix * glm::mat4(glm::mat3(_renderData.viewMatrix));

	if (transformationComponent)
	{
		mvpMatrix *= glm::mat4_cast(transformationComponent->rotation);
	}

	glm::mat4 invTransform = glm::inverse(mvpMatrix);

	uInverseModelViewProjectionB.set(invTransform);
	uCurrentToPrevTransformB.set(prevTransform * invTransform);

	prevTransform = mvpMatrix;

	fullscreenTriangle->getSubMesh()->render();
	//glDrawArrays(GL_TRIANGLES, 0, 3);
}

void SceneRenderer::renderEnvironmentLight(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const Effects &_effects)
{
	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, ssaoTextureA);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _level->environment.environmentProbe->getIrradianceMap()->getId());
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _level->environment.environmentProbe->getReflectanceMap()->getId());
	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, brdfLUT);
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D, gLightColorTextures[(currentLightColorTexture + 1) % 2]);

	environmentLightPassShader->bind();

	uInverseViewE.set(_renderData.invViewMatrix);
	uProjectionE.set(_renderData.projectionMatrix);
	uInverseProjectionE.set(_renderData.invProjectionMatrix);
	uSsaoE.set(_effects.ambientOcclusion != AmbientOcclusion::OFF);
	uUseSsrE.set(_effects.screenSpaceReflections.enabled);

	static glm::mat4 prevViewProjection;

	uReProjectionE.set(prevViewProjection * _renderData.invViewProjectionMatrix);
	prevViewProjection = _renderData.viewProjectionMatrix;

	if (!_level->lights.directionalLights.empty())
	{
		std::shared_ptr<DirectionalLight> directionalLight = _level->lights.directionalLights[0];
		directionalLight->updateViewValues(_renderData.viewMatrix);
		if (directionalLight->isRenderShadows())
		{
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D_ARRAY, directionalLight->getShadowMap());
		}
		uDirectionalLightE.set(directionalLight);
		uRenderDirectionalLightE.set(true);
		uShadowsEnabledE.set(_renderData.shadows);
	}
	else
	{
		uRenderDirectionalLightE.set(false);
	}

	fullscreenTriangle->getSubMesh()->render();
	//glDrawArrays(GL_TRIANGLES, 0, 3);
}

void SceneRenderer::renderDirectionalLights(const RenderData &_renderData, const std::shared_ptr<Level> &_level)
{
	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	directionalLightShader->bind();

	uInverseViewD.set(_renderData.invViewMatrix);
	uInverseProjectionD.set(_renderData.invProjectionMatrix);
	uShadowsEnabledD.set(_renderData.shadows);

	for (size_t i = _level->environment.skyboxEntity ? 1 : 0; i < _level->lights.directionalLights.size(); ++i)
	{
		std::shared_ptr<DirectionalLight> directionalLight = _level->lights.directionalLights[i];
		directionalLight->updateViewValues(_renderData.viewMatrix);
		if (directionalLight->isRenderShadows())
		{
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D_ARRAY, directionalLight->getShadowMap());
		}

		uDirectionalLightD.set(directionalLight);
		fullscreenTriangle->getSubMesh()->render();
		//glDrawArrays(GL_TRIANGLES, 0, 3);
	}
}

void SceneRenderer::renderPointLights(const RenderData &_renderData, const std::shared_ptr<Level> &_level)
{
	pointLightMesh->getSubMesh()->enableVertexAttribArrays();

	pointLightPassShader->bind();

	uInverseViewP.set(_renderData.invViewMatrix);
	uInverseProjectionP.set(_renderData.invProjectionMatrix);
	uShadowsEnabledP.set(_renderData.shadows);
	uViewportSizeP.set(glm::vec2(_renderData.resolution.first, _renderData.resolution.second));

	for (std::shared_ptr<PointLight> pointLight : _level->lights.pointLights)
	{
		if (!_renderData.frustum.testSphere(pointLight->getBoundingSphere()))
		{
			continue;
		}

		pointLight->updateViewValues(_renderData.viewMatrix);

		if (pointLight->isRenderShadows())
		{
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_CUBE_MAP, pointLight->getShadowMap());
		}

		uModelViewProjectionP.set(_renderData.viewProjectionMatrix * glm::translate(pointLight->getPosition()) * glm::scale(glm::vec3(pointLight->getRadius())));
		uPointLightP.set(pointLight);
		pointLightMesh->getSubMesh()->render();
	}
}

void SceneRenderer::renderSpotLights(const RenderData &_renderData, const std::shared_ptr<Level> &_level)
{
	spotLightMesh->getSubMesh()->enableVertexAttribArrays();

	spotLightPassShader->bind();

	uInverseViewS.set(_renderData.invViewMatrix);
	uInverseProjectionS.set(_renderData.invProjectionMatrix);
	uShadowsEnabledS.set(_renderData.shadows);
	uViewportSizeS.set(glm::vec2(_renderData.resolution.first, _renderData.resolution.second));

	for (std::shared_ptr<SpotLight> spotLight : _level->lights.spotLights)
	{
		if (!_renderData.frustum.testSphere(spotLight->getBoundingSphere()))
		{
			continue;
		}

		spotLight->updateViewValues(_renderData.viewMatrix);

		if (spotLight->isRenderShadows())
		{
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, spotLight->getShadowMap());
		}
		if (spotLight->isProjector())
		{
			assert(spotLight->getProjectionTexture());
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, spotLight->getProjectionTexture()->getId());
		}

		// scale a bit larger to correct for proxy geometry not being exactly round
		float scale = (glm::tan(spotLight->getOuterAngle()) + 0.1f) * spotLight->getRadius();

		const glm::vec3 defaultDirection = glm::vec3(0.0f, -1.0f, 0.0f);

		uModelViewProjectionS.set(_renderData.viewProjectionMatrix
			* glm::translate(spotLight->getPosition())
			* glm::mat4_cast(glm::rotation(defaultDirection, spotLight->getDirection()))
			* glm::scale(glm::vec3(scale, spotLight->getRadius(), scale)));
		uSpotLightS.set(spotLight);
		spotLightMesh->getSubMesh()->render();
	}
}

void SceneRenderer::renderTransparentGeometry(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const Scene &_scene)
{
	if (!_scene.getTransparencyCount())
	{
		return;
	}

	transparencyShader->bind();

	if (!_level->lights.directionalLights.empty())
	{
		if (_level->lights.directionalLights[0]->isRenderShadows())
		{
			glActiveTexture(GL_TEXTURE10);
			glBindTexture(GL_TEXTURE_2D_ARRAY, _level->lights.directionalLights[0]->getShadowMap());
		}
		uRenderDirectionalLightT.set(true);
		uDirectionalLightT.set(_level->lights.directionalLights[0]);
	}
	else
	{
		uRenderDirectionalLightT.set(false);
	}
	uShadowsEnabledT.set(_renderData.shadows);
	uCamPosT.set(_renderData.cameraPosition);
	uViewMatrixT.set(_renderData.viewMatrix);

	const std::vector<std::unique_ptr<EntityRenderData>> &data = _scene.getData();

	std::shared_ptr<SubMesh> currentMesh = nullptr;
	bool enabledMesh = false;

	for (std::size_t i = 0; i < data.size(); ++i)
	{
		const std::unique_ptr<EntityRenderData> &entityRenderData = data[i];

		// skip this iteration if its supposed to be rendered with another method or does not have sufficient components
		if (entityRenderData->customOpaqueShaderComponent ||
			entityRenderData->customTransparencyShaderComponent ||
			!entityRenderData->transparencyComponent ||
			!entityRenderData->modelComponent ||
			!entityRenderData->transformationComponent)
		{
			continue;
		}

		if (currentMesh != entityRenderData->mesh)
		{
			currentMesh = entityRenderData->mesh;
			enabledMesh = false;
		}

		// skip this mesh if its not transparent
		if (entityRenderData->transparencyComponent && !ContainerUtility::contains(entityRenderData->transparencyComponent->transparentSubMeshes, currentMesh))
		{
			continue;
		}

		// skip this iteration if the mesh is not yet valid
		if (!currentMesh || !currentMesh->isValid())
		{
			continue;
		}

		if (!enabledMesh)
		{
			enabledMesh = true;
			currentMesh->enableVertexAttribArrays();
		}

		// we're good to go: render this mesh-entity instance
		uMaterialT.set(entityRenderData->material);
		entityRenderData->material->bindTextures();

		glm::mat4 modelMatrix = glm::translate(entityRenderData->transformationComponent->position)
			* glm::mat4_cast(entityRenderData->transformationComponent->rotation)
			* glm::scale(glm::vec3(entityRenderData->transformationComponent->scale));

		int rows = 1;
		int columns = 1;
		glm::vec2 textureOffset;
		TextureAtlasIndexComponent *textureAtlasComponent = entityRenderData->textureAtlasIndexComponent;
		if (textureAtlasComponent && ContainerUtility::contains(textureAtlasComponent->meshToIndexMap, currentMesh))
		{
			rows = textureAtlasComponent->rows;
			columns = textureAtlasComponent->columns;
			int texPos = textureAtlasComponent->meshToIndexMap[currentMesh];
			int col = texPos % columns;
			int row = texPos / columns;
			textureOffset = glm::vec2((float)col / columns, (float)row / rows);
		}

		glm::mat4 mvpTransformation = _renderData.viewProjectionMatrix * modelMatrix;
		const float cameraMovementStrength = 0.15f;
		glm::mat4 prevTransformation = glm::mix(_renderData.invJitter * _renderData.viewProjectionMatrix, _renderData.prevInvJitter * _renderData.prevViewProjectionMatrix, cameraMovementStrength) * entityRenderData->transformationComponent->prevTransformation;

		uAtlasDataT.set(glm::vec4(1.0f / columns, 1.0f / rows, textureOffset));
		uModelMatrixT.set(modelMatrix);
		uModelViewProjectionMatrixT.set(mvpTransformation);
		uPrevTransformT.set(prevTransformation);
		uCurrTransformT.set(_renderData.invJitter * mvpTransformation);

		entityRenderData->transformationComponent->prevTransformation = modelMatrix;

		if (entityRenderData->outlineComponent)
		{
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
			glStencilMask(0xFF);
		}

		currentMesh->render();

		if (entityRenderData->outlineComponent)
		{
			glStencilMask(0x00);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		}
	}
}

void SceneRenderer::renderOutlines(const RenderData &_renderData, const Scene &_scene)
{
	if (!_scene.getOutlineCount())
	{
		_scene.getOutlineCount();
	}

	outlineShader->bind();
	glStencilMask(0x00);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

	const std::vector<std::unique_ptr<EntityRenderData>> &data = _scene.getData();

	std::shared_ptr<SubMesh> currentMesh = nullptr;
	bool enabledMesh = false;

	for (std::size_t i = 0; i < data.size(); ++i)
	{
		const std::unique_ptr<EntityRenderData> &entityRenderData = data[i];

		// skip this iteration if its supposed to be rendered with another method or does not have sufficient components
		if (!entityRenderData->outlineComponent ||
			!entityRenderData->modelComponent ||
			!entityRenderData->transformationComponent)
		{
			continue;
		}

		if (currentMesh != entityRenderData->mesh)
		{
			currentMesh = entityRenderData->mesh;
			enabledMesh = false;
		}

		// skip this iteration if the mesh is not yet valid
		if (!currentMesh || !currentMesh->isValid())
		{
			continue;
		}

		if (!enabledMesh)
		{
			enabledMesh = true;
			currentMesh->enableVertexAttribArrays();
		}

		// we're good to go: render this mesh-entity instance
		glm::mat4 modelMatrix = glm::translate(entityRenderData->transformationComponent->position)
			* glm::mat4_cast(entityRenderData->transformationComponent->rotation)
			* glm::scale(glm::vec3(entityRenderData->transformationComponent->scale * entityRenderData->outlineComponent->scaleMultiplier));

		uOutlineColorO.set(entityRenderData->outlineComponent->outlineColor);
		uModelViewProjectionMatrixO.set(_renderData.viewProjectionMatrix * modelMatrix);

		currentMesh->render();
	}

	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
}

void SceneRenderer::renderCustomGeometry(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const Scene &_scene, bool _opaque)
{
	if (_opaque && !_scene.getCustomOpaqueCount() || !_opaque && !_scene.getCustomTransparencyCount())
	{
		return;
	}

	const std::vector<std::unique_ptr<EntityRenderData>> &data = _scene.getData();

	std::shared_ptr<SubMesh> currentMesh = nullptr;
	bool enabledMesh = false;

	for (std::size_t i = 0; i < data.size(); ++i)
	{
		const std::unique_ptr<EntityRenderData> &entityRenderData = data[i];

		// skip this iteration if its supposed to be rendered with another method or does not have sufficient components
		if (!entityRenderData->customOpaqueShaderComponent && !entityRenderData->customTransparencyShaderComponent)
		{
			continue;
		}

		if (currentMesh != entityRenderData->mesh)
		{
			currentMesh = entityRenderData->mesh;
			enabledMesh = false;
		}

		// skip this iteration if the mesh is not yet valid
		if (!currentMesh || !currentMesh->isValid())
		{
			continue;
		}

		if (!enabledMesh)
		{
			enabledMesh = true;
			currentMesh->enableVertexAttribArrays();
		}

		// we're good to go: render this mesh-entity instance
		if (_opaque && entityRenderData->customOpaqueShaderComponent)
		{
			entityRenderData->customOpaqueShaderComponent->opaqueShader->bind();
			entityRenderData->customOpaqueShaderComponent->renderOpaque(_renderData, _level, entityRenderData);
		}
		else if (!_opaque && entityRenderData->customTransparencyShaderComponent)
		{
			entityRenderData->customTransparencyShaderComponent->transparencyShader->bind();
			entityRenderData->customTransparencyShaderComponent->renderTransparency(_renderData, _level, entityRenderData);
		}
	}
}

void SceneRenderer::renderSsaoTexture(const RenderData &_renderData, const Effects &_effects)
{
	float sharpness = 1.0f;
	float kernelRadius = 3.0f;
	switch (_effects.ambientOcclusion)
	{
	case AmbientOcclusion::SSAO_ORIGINAL:
	{
		fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFbo);
		glViewport(0, 0, _renderData.resolution.first, _renderData.resolution.second);

		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);

		ssaoOriginalShader->bind();

		fullscreenTriangle->getSubMesh()->render();

		ssaoBilateralBlurShader->bind();

		glDrawBuffer(GL_COLOR_ATTACHMENT1);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, ssaoTextureA);

		uSharpnessAOBB.set(sharpness);
		uKernelRadiusAOBB.set(kernelRadius);
		uInvResolutionDirectionAOBB.set(glm::vec2(1.0f / _renderData.resolution.first, 0.0));

		fullscreenTriangle->getSubMesh()->render();

		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, ssaoTextureB);

		uInvResolutionDirectionAOBB.set(glm::vec2(0.0, 1.0f / _renderData.resolution.second));

		fullscreenTriangle->getSubMesh()->render();

		break;
	}
	case AmbientOcclusion::SSAO:
	{
		std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
		static std::default_random_engine generator;
		static std::vector<glm::vec3> ssaoKernel;
		static bool generateKernel = true;
		static unsigned int currentKernelSize = 16;
		if (currentKernelSize != _effects.ssao.kernelSize)
		{
			currentKernelSize = _effects.ssao.kernelSize;
			generateKernel = true;
		}
		if (generateKernel)
		{
			ssaoKernel.clear();
			for (unsigned int i = 0; i < currentKernelSize; ++i)
			{
				glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
				sample = glm::normalize(sample);
				sample *= randomFloats(generator);
				float scale = float(i) / currentKernelSize;

				// scale samples s.t. they're more aligned to center of kernel
				scale = glm::mix(0.1f, 1.0f, scale * scale);
				sample *= scale;
				ssaoKernel.push_back(sample);
			}
		}

		fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFbo);
		glViewport(0, 0, _renderData.resolution.first, _renderData.resolution.second);

		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);

		ssaoShader->bind();
		uViewAO.set(_renderData.viewMatrix);
		uProjectionAO.set(_renderData.projectionMatrix);
		uInverseProjectionAO.set(_renderData.invProjectionMatrix);
		uKernelSizeAO.set((int)currentKernelSize);
		uRadiusAO.set(_effects.ssao.radius);
		uBiasAO.set(_effects.ssao.bias);
		uStrengthAO.set(_effects.ssao.strength);
		if (generateKernel)
		{
			generateKernel = false;
			for (unsigned int i = 0; i < currentKernelSize; ++i)
			{
				ssaoShader->setUniform(uSamplesAO[i], ssaoKernel[i]);
			}
		}

		fullscreenTriangle->getSubMesh()->render();
		//glDrawArrays(GL_TRIANGLES, 0, 3);

		ssaoBilateralBlurShader->bind();

		glDrawBuffer(GL_COLOR_ATTACHMENT1);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, ssaoTextureA);

		uSharpnessAOBB.set(_effects.ssao.blurSharpness);
		uKernelRadiusAOBB.set(_effects.ssao.blurRadius);
		uInvResolutionDirectionAOBB.set(glm::vec2(1.0f / _renderData.resolution.first, 0.0));

		fullscreenTriangle->getSubMesh()->render();

		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, ssaoTextureB);

		uInvResolutionDirectionAOBB.set(glm::vec2(0.0, 1.0f / _renderData.resolution.second));

		fullscreenTriangle->getSubMesh()->render();
		break;
	}
	case AmbientOcclusion::HBAO:
	{
		fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFbo);
		glViewport(0, 0, _renderData.resolution.first, _renderData.resolution.second);

		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, noiseTexture2);

		float aspectRatio = _renderData.resolution.second / (float)_renderData.resolution.first;
		float fovy = 2.0f * glm::atan(glm::tan(glm::radians(window->getFieldOfView()) * 0.5f) * aspectRatio);
		glm::vec2 focalLength;
		focalLength.x = 1.0f / tanf(fovy * 0.5f) * aspectRatio;
		focalLength.y = 1.0f / tanf(fovy * 0.5f);

		glm::vec2 res(_renderData.resolution.first, _renderData.resolution.second);
		float radius = 0.3f;

		hbaoShader->bind();
		uFocalLengthHBAO.set(focalLength);
		uInverseProjectionHBAO.set(_renderData.invProjectionMatrix);
		uAOResHBAO.set(res);
		uInvAOResHBAO.set(1.0f / res);
		uNoiseScaleHBAO.set(res * 0.25f);
		uStrengthHBAO.set(_effects.hbao.strength);
		uRadiusHBAO.set(_effects.hbao.radius);
		uRadius2HBAO.set(_effects.hbao.radius * _effects.hbao.radius);
		uNegInvR2HBAO.set(-1.0f / (_effects.hbao.radius * _effects.hbao.radius));
		uTanBiasHBAO.set(_effects.hbao.angleBias);
		uMaxRadiusPixelsHBAO.set(_effects.hbao.maxRadiusPixels);
		uNumDirectionsHBAO.set((float)_effects.hbao.directions);
		uNumStepsHBAO.set((float)_effects.hbao.steps);

		fullscreenTriangle->getSubMesh()->render();

		ssaoBilateralBlurShader->bind();

		glDrawBuffer(GL_COLOR_ATTACHMENT1);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, ssaoTextureA);

		uSharpnessAOBB.set(_effects.hbao.blurSharpness);
		uKernelRadiusAOBB.set(_effects.hbao.blurRadius);
		uInvResolutionDirectionAOBB.set(glm::vec2(1.0f / _renderData.resolution.first, 0.0));

		fullscreenTriangle->getSubMesh()->render();

		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, ssaoTextureB);

		uInvResolutionDirectionAOBB.set(glm::vec2(0.0, 1.0f / _renderData.resolution.second));

		fullscreenTriangle->getSubMesh()->render();

		break;
	}
	default:
		assert(false);
	}
}

void SceneRenderer::createBrdfLUT()
{
	std::shared_ptr<ShaderProgram> brdfShader = ShaderProgram::createShaderProgram("Resources/Shaders/Renderer/brdf.comp");

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

bool SceneRenderer::cullAABB(const glm::mat4 &_mvp, const AxisAlignedBoundingBox &_aabb)
{
	return false;
	// TODO: properly implement this


	glm::vec4 points[8] =
	{
		glm::vec4(_aabb.min, 1.0), // xyz
		glm::vec4(_aabb.max.x, _aabb.min.y, _aabb.min.z, 1.0), // Xyz
		glm::vec4(_aabb.min.x, _aabb.max.y, _aabb.min.z, 1.0), // xYz
		glm::vec4(_aabb.max.x, _aabb.max.y, _aabb.min.z, 1.0),// XYz
		glm::vec4(_aabb.min.x, _aabb.min.y, _aabb.max.z, 1.0), // xyZ
		glm::vec4(_aabb.max.x, _aabb.min.y, _aabb.max.z, 1.0), // XyZ
		glm::vec4(_aabb.min.x, _aabb.max.y, _aabb.max.z, 1.0), // xYZ
		glm::vec4(_aabb.max, 1.0) // XYZ
	};

	// transform to clipspace and normalize
	for (size_t i = 0; i < 8; ++i)
	{
		points[i] = _mvp * points[i];
		points[i] /= points[i].w;
	}

	// check each side
	for (size_t i = 0; i < 6; ++i)
	{
		bool inside = false;

		// check each point
		for (size_t j = 0; j < 8; ++j)
		{
			// first half of sides is positive, second half is negative
			if (i < 3)
			{
				if (points[j][i % 3] <= 1.0)
				{
					inside = true;
					break;
				}
			}
			else
			{
				if (points[j][i % 3] >= -1.0)
				{
					inside = true;
					break;
				}
			}
		}

		if (!inside)
		{
			return true;
		}
	}

	return false;
}