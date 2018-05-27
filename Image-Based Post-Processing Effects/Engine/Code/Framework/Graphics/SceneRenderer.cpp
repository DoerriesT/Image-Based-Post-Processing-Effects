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
#include "Utilities\Utility.h"
#include "Engine.h"
#include ".\..\..\EntityComponentSystem\EntityManager.h"
#include "ShaderProgram.h"
#include ".\..\..\Graphics\Camera.h"
#include ".\..\..\Window.h"
#include ".\..\..\Level.h"
#include ".\..\..\Graphics\Mesh.h"
#include ".\..\..\Graphics\EntityRenderData.h"
#include "RenderData.h"
#include ".\..\..\Graphics\Scene.h"
#include ".\..\..\Graphics\Effects.h"
#include ".\..\..\Graphics\Texture.h"

//#define SPHERES

#ifdef SPHERES
void renderSphere();
#endif // SPHERES

const int N = 256;
const int log2N = glm::log2(N);
const int L = 500;
const float A = 4.0f;
const glm::vec2 windDirection = glm::normalize(glm::vec2(1.0, 1.0));
const float windSpeed = 26.0f;

float calculateLightScale(const glm::vec3 &_color)
{
	static const float MIN_THRESHOLD = 1.0f / 256.0f;
	float maxColor = glm::max(_color.r, glm::max(_color.g, _color.b));
	return sqrt(maxColor / MIN_THRESHOLD);
}

SceneRenderer::SceneRenderer(std::shared_ptr<Window> _window)
	:window(_window)
{
}

SceneRenderer::~SceneRenderer()
{
	// delete water mesh
	glBindVertexArray(waterVAO);
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &waterEBO);
	glDeleteBuffers(1, &waterVBO);
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &waterVAO);

	GLuint textures[] = { gAlbedoTexture, gNormalTexture, gMRASTexture, gDepthStencilTexture, gLightColorTextures[0], gLightColorTextures[1], gVelocityTexture };
	glDeleteTextures(sizeof(textures) / sizeof(GLuint), textures);
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
	tildeH0kShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Water/tildeH0k.frag");
	tildeHktShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Water/tildeHkt.frag");
	butterflyPrecomputeShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Water/butterflyPrecompute.frag");
	butterflyComputeShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Water/butterflyCompute.frag");
	inversePermuteShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Water/inversePermute.frag");
	waterNormalShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Water/normal.frag");
	waterShader = ShaderProgram::createShaderProgram("Resources/Shaders/Water/water.vert", "Resources/Shaders/Water/Water.frag");

	// create uniforms

	// gBufferPass uniforms
	uMaterialG.create(gBufferPassShader);
	uModelMatrixG.create(gBufferPassShader);
	uModelViewProjectionMatrixG.create(gBufferPassShader);
	uPrevTransformG.create(gBufferPassShader);
	uAtlasDataG.create(gBufferPassShader);
	uVelG.create(gBufferPassShader);
	uExposureTimeG.create(gBufferPassShader);

	// outline uniforms
	uModelViewProjectionMatrixO.create(outlineShader);
	uOutlineColorO.create(outlineShader);

	// environmentLightPass uniforms
	uAlbedoMapE.create(environmentLightPassShader);
	uNormalMapE.create(environmentLightPassShader);
	uMetallicRoughnessAoMapE.create(environmentLightPassShader);
	uDepthMapE.create(environmentLightPassShader);
	uSsaoMapE.create(environmentLightPassShader);
	uInverseProjectionE.create(environmentLightPassShader);
	uInverseViewE.create(environmentLightPassShader);
	uCamPosE.create(environmentLightPassShader);
	uIrradianceMapE.create(environmentLightPassShader);
	uPrefilterMapE.create(environmentLightPassShader);
	uBrdfLUTE.create(environmentLightPassShader);
	uDirectionalLightE.create(environmentLightPassShader);
	uShadowsEnabledE.create(environmentLightPassShader);
	uRenderDirectionalLightE.create(environmentLightPassShader);
	uSsaoE.create(environmentLightPassShader);
	uPrevFrameE.create(environmentLightPassShader);
	uViewE.create(environmentLightPassShader);
	uProjectionE.create(environmentLightPassShader);
	uPrevViewProjectionE.create(environmentLightPassShader);
	uUseSsrE.create(environmentLightPassShader);


	// pointLightPass uniforms
	uModelViewProjectionP.create(pointLightPassShader);
	uAlbedoMapP.create(pointLightPassShader);
	uNormalMapP.create(pointLightPassShader);
	uMetallicRoughnessAoMapP.create(pointLightPassShader);
	uDepthMapP.create(pointLightPassShader);
	uPointLightP.create(pointLightPassShader);
	uInverseProjectionP.create(pointLightPassShader);
	uInverseViewP.create(pointLightPassShader);
	uCamPosP.create(pointLightPassShader);
	uShadowsEnabledP.create(pointLightPassShader);
	uViewportSizeP.create(pointLightPassShader);

	// spotLightPass uniforms
	uModelViewProjectionS.create(spotLightPassShader);
	uAlbedoMapS.create(spotLightPassShader);
	uNormalMapS.create(spotLightPassShader);
	uMetallicRoughnessAoMapS.create(spotLightPassShader);
	uDepthMapS.create(spotLightPassShader);
	uSpotLightS.create(spotLightPassShader);
	uInverseViewS.create(spotLightPassShader);
	uInverseProjectionS.create(spotLightPassShader);
	uCamPosS.create(spotLightPassShader);
	uShadowsEnabledS.create(spotLightPassShader);
	uViewportSizeS.create(spotLightPassShader);

	// directionalLightPass uniforms
	uAlbedoMapD.create(directionalLightShader);
	uNormalMapD.create(directionalLightShader);
	uMetallicRoughnessAoMapD.create(directionalLightShader);
	uDepthMapD.create(directionalLightShader);
	uDirectionalLightD.create(directionalLightShader);
	uInverseViewD.create(directionalLightShader);
	uInverseProjectionD.create(directionalLightShader);
	uCamPosD.create(directionalLightShader);
	uShadowsEnabledD.create(directionalLightShader);

	// skybox uniforms
	uInverseModelViewProjectionB.create(skyboxShader);
	uAlbedoMapB.create(skyboxShader);
	uColorB.create(skyboxShader);
	uHasAlbedoMapB.create(skyboxShader);
	uCurrentToPrevTransformB.create(skyboxShader);

	// transparency uniforms
	uPrevTransformT.create(transparencyShader);
	uModelViewProjectionMatrixT.create(transparencyShader);
	uModelMatrixT.create(transparencyShader);
	uAtlasDataT.create(transparencyShader);
	uMaterialT.create(transparencyShader);
	uDirectionalLightT.create(transparencyShader);
	uRenderDirectionalLightT.create(transparencyShader);
	uCamPosT.create(transparencyShader);
	uShadowsEnabledT.create(transparencyShader);
	uIrradianceMapT.create(transparencyShader);
	uPrefilterMapT.create(transparencyShader);
	uBrdfLUTT.create(transparencyShader);

	// ssao
	uDepthTextureAO.create(ssaoShader);
	uNormalTextureAO.create(ssaoShader);
	uNoiseTextureAO.create(ssaoShader);
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

	// ssao original
	uDepthTextureAOO.create(ssaoOriginalShader);
	uNoiseTextureAOO.create(ssaoOriginalShader);

	// ssao blur
	uInputTextureAOB.create(ssaoBlurShader);
	uBlurSizeAOB.create(ssaoBlurShader);

	// tildeh0k
	uNoiseR0TextureH0.create(tildeH0kShader);
	uNoiseI0TextureH0.create(tildeH0kShader);
	uNoiseR1TextureH0.create(tildeH0kShader);
	uNoiseI1TextureH0.create(tildeH0kShader);
	uNH0.create(tildeH0kShader);
	uLH0.create(tildeH0kShader);
	uAH0.create(tildeH0kShader);
	uWindDirectionH0.create(tildeH0kShader);
	uWindSpeedH0.create(tildeH0kShader);

	// tildehkt
	uTildeH0kTextureHT.create(tildeHktShader);
	uTildeH0minusKTextureHT.create(tildeHktShader);
	uNHT.create(tildeHktShader);
	uLHT.create(tildeHktShader);
	uTimeHT.create(tildeHktShader);

	// butterfly precompute
	for (int i = 0; i < N; ++i)
	{
		uJBP.push_back(butterflyPrecomputeShader->createUniform(std::string("uJ") + "[" + std::to_string(i) + "]"));
	}
	uNBP.create(butterflyPrecomputeShader);

	// butterfly compute
	uButterflyTextureBC.create(butterflyComputeShader);
	uInputXTextureBC.create(butterflyComputeShader);
	uInputYTextureBC.create(butterflyComputeShader);
	uInputZTextureBC.create(butterflyComputeShader);
	uNBC.create(butterflyComputeShader);
	uStageBC.create(butterflyComputeShader);
	uStagesBC.create(butterflyComputeShader);
	uDirectionBC.create(butterflyComputeShader);

	// inverse / permute
	uInputXTextureIP.create(inversePermuteShader);
	uInputYTextureIP.create(inversePermuteShader);
	uInputZTextureIP.create(inversePermuteShader);
	uNIP.create(inversePermuteShader);
	uChoppinessIP.create(inversePermuteShader);

	// water
	uNormalTextureW.create(waterShader);
	uDisplacementTextureW.create(waterShader);
	uFoamTextureW.create(waterShader);
	uProjectionW.create(waterShader);
	uViewW.create(waterShader);
	uCamPosW.create(waterShader);
	uTexCoordShiftW.create(waterShader);
	uEnvironmentTextureW.create(waterShader);
	uUseEnvironmentW.create(waterShader);
	uWaterLevelW.create(waterShader);
	uLightDirW.create(waterShader);
	uLightColorW.create(waterShader);

	// water normal
	uDisplacementTextureN.create(waterNormalShader);
	uNormalStrengthN.create(waterNormalShader);

	// create FBO
	glGenFramebuffers(1, &gBufferFBO);
	glGenFramebuffers(1, &fftFbo);
	glGenFramebuffers(1, &twiddleIndicesFbo);
	glGenFramebuffers(1, &waterFbo);
	glGenFramebuffers(1, &ssaoFbo);

	auto res = std::make_pair(window->getWidth(), window->getHeight());
	createFboAttachments(res);
	createWaterAttachments();
	createSsaoAttachments(res);

	pointLightMesh = Mesh::createMesh("Resources/Models/pointlight.obj", true);
	spotLightMesh = Mesh::createMesh("Resources/Models/spotlight.obj", true);
	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.obj", true);

	createWaterPlane(waterGridDimensions, waterVBO, waterVAO, waterEBO);
}

void SceneRenderer::render(const RenderData &_renderData, const Scene &_scene, const std::shared_ptr<Level> &_level, const Effects &_effects)
{
	// switch current result texture
	currentLightColorTexture = (currentLightColorTexture + 1) % 2;

	const GLenum firstPassDrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 , lightColorAttachments[currentLightColorTexture] };
	const GLenum secondPassDrawBuffers[] = { lightColorAttachments[currentLightColorTexture], GL_COLOR_ATTACHMENT3 };
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

	// render ordinary geometry
	renderGeometry(_renderData, _scene);

	// render custom opaque geometry
	renderCustomGeometry(_renderData, _level, _scene, true);

	// disable stencil testing for now; we will continue writing to the stencil when we do transparency
	glDisable(GL_STENCIL_TEST);

	// calculate matrix inputs for lighting shaders
	glm::mat4 invView = glm::inverse(_renderData.viewMatrix);
	glm::mat4 invProj = glm::inverse(_renderData.projectionMatrix);

	// compute water textures
	static bool precomputed = false;
	if (!precomputed && _level->water.enabled)
	{
		glDisable(GL_CULL_FACE);
		precomputeFftTextures();
		precomputed = true;
		glEnable(GL_CULL_FACE);
	}
	if (_level->water.enabled)
	{
		glDisable(GL_CULL_FACE);
		computeFft();
		glEnable(GL_CULL_FACE);
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
	if (_effects.ssao.enabled)
	{
		glDisable(GL_CULL_FACE);
		renderSsaoTexture(_renderData, invProj, _effects);
		glEnable(GL_CULL_FACE);
	}

	if (_effects.ssao.enabled || _level->water.enabled)
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
		renderEnvironmentLight(_renderData, _level, invView, invProj, _effects);
	}
	// render remaining directional lights
	if (_level->lights.directionalLights.size() > 1 || !_level->environment.skyboxEntity)
	{
		renderDirectionalLights(_renderData, _level, invView, invProj);
	}

	//glEnable(GL_DEPTH_TEST);

	// cull front faces because otherwise entering a light sphere will make the light disappear
	glCullFace(GL_FRONT);

	// render point- and spotlights
	if (!_level->lights.pointLights.empty() || !_level->lights.spotLights.empty())
	{
		if (!_level->lights.pointLights.empty())
		{
			renderPointLights(_renderData, _level, invView, invProj);
		}
		if (!_level->lights.spotLights.empty())
		{
			renderSpotLights(_renderData, _level, invView, invProj);
		}
	}

	glEnable(GL_DEPTH_TEST);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);



	glDepthMask(GL_TRUE);

	if (_level->water.enabled)
	{
		glDisable(GL_CULL_FACE);
		renderWater(_renderData, _level);
		glEnable(GL_CULL_FACE);
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
}

void SceneRenderer::resize(const std::pair<unsigned int, unsigned int> &_resolution)
{
	GLuint textures[] = { gAlbedoTexture, gNormalTexture, gMRASTexture, gDepthStencilTexture, gLightColorTextures[0], gLightColorTextures[1], gVelocityTexture, ssaoTextureA, ssaoTextureB };
	glDeleteTextures(sizeof(textures) / sizeof(GLuint), textures);
	createFboAttachments(_resolution);
}

GLuint SceneRenderer::getColorTexture() const
{
	return gLightColorTextures[currentLightColorTexture];
}

GLuint SceneRenderer::getDepthStencilTexture() const
{
	return gDepthStencilTexture;
}

GLuint SceneRenderer::getVelocityTexture() const
{
	return gVelocityTexture;
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, _resolution.first, _resolution.second, 0, GL_RG, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gVelocityTexture, 0);

	glGenTextures(1, &gLightColorTextures[0]);
	glBindTexture(GL_TEXTURE_2D, gLightColorTextures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, _resolution.first, _resolution.second, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, gLightColorTextures[0], 0);

	glGenTextures(1, &gLightColorTextures[1]);
	glBindTexture(GL_TEXTURE_2D, gLightColorTextures[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, _resolution.first, _resolution.second, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, gLightColorTextures[1], 0);

	lightColorAttachments[0] = GL_COLOR_ATTACHMENT4;
	lightColorAttachments[1] = GL_COLOR_ATTACHMENT5;

	glGenTextures(1, &gDepthStencilTexture);
	glBindTexture(GL_TEXTURE_2D, gDepthStencilTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, _resolution.first, _resolution.second, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
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

void SceneRenderer::createWaterAttachments()
{
	glBindFramebuffer(GL_FRAMEBUFFER, fftFbo);

	glGenTextures(1, &tildeH0kTexture);
	glBindTexture(GL_TEXTURE_2D, tildeH0kTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, N, N, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tildeH0kTexture, 0);

	glGenTextures(1, &tildeH0minusKTexture);
	glBindTexture(GL_TEXTURE_2D, tildeH0minusKTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, N, N, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, tildeH0minusKTexture, 0);

	glGenTextures(1, &tildeHktDxTexture);
	glBindTexture(GL_TEXTURE_2D, tildeHktDxTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, N, N, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, tildeHktDxTexture, 0);

	glGenTextures(1, &tildeHktDyTexture);
	glBindTexture(GL_TEXTURE_2D, tildeHktDyTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, N, N, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, tildeHktDyTexture, 0);

	glGenTextures(1, &tildeHktDzTexture);
	glBindTexture(GL_TEXTURE_2D, tildeHktDzTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, N, N, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, tildeHktDzTexture, 0);

	glGenTextures(1, &pingPongTextureA);
	glBindTexture(GL_TEXTURE_2D, pingPongTextureA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, N, N, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, pingPongTextureA, 0);

	glGenTextures(1, &pingPongTextureB);
	glBindTexture(GL_TEXTURE_2D, pingPongTextureB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, N, N, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, pingPongTextureB, 0);

	glGenTextures(1, &pingPongTextureC);
	glBindTexture(GL_TEXTURE_2D, pingPongTextureC);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, N, N, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT7, GL_TEXTURE_2D, pingPongTextureC, 0);


	glBindFramebuffer(GL_FRAMEBUFFER, twiddleIndicesFbo);

	glGenTextures(1, &twiddleIndicesTexture);
	glBindTexture(GL_TEXTURE_2D, twiddleIndicesTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, log2N, N, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, twiddleIndicesTexture, 0);


	glBindFramebuffer(GL_FRAMEBUFFER, waterFbo);

	glGenTextures(1, &waterDisplacementFoldingTexture);
	glBindTexture(GL_TEXTURE_2D, waterDisplacementFoldingTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, N, N, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, waterDisplacementFoldingTexture, 0);

	glGenTextures(1, &waterNormalTexture);
	glBindTexture(GL_TEXTURE_2D, waterNormalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, N, N, 0, GL_RG, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, waterNormalTexture, 0);
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

	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFbo);

	glGenTextures(1, &ssaoTextureA);
	glBindTexture(GL_TEXTURE_2D, ssaoTextureA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, _resolution.first / 2, _resolution.second / 2, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoTextureA, 0);

	glGenTextures(1, &ssaoTextureB);
	glBindTexture(GL_TEXTURE_2D, ssaoTextureB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, _resolution.first / 2, _resolution.second / 2, 0, GL_RED, GL_FLOAT, NULL);
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

	std::shared_ptr<Mesh> currentMesh = nullptr;
	bool enabledMesh = false;

	for (std::size_t i = 0; i < data.size(); ++i)
	{
		const std::unique_ptr<EntityRenderData> &entityRenderData = data[i];

		// skip this iteration if its supposed to be rendered with another method or does not have sufficient components
		if (entityRenderData->customOpaqueShaderComponent ||
			entityRenderData->customTransparencyShaderComponent ||
			entityRenderData->transparencyComponent ||
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
		uMaterialG.set(entityRenderData->material);
		entityRenderData->material->bindTextures();

		glm::mat4 modelMatrix = glm::translate(entityRenderData->transformationComponent->position)
			* glm::mat4_cast(entityRenderData->transformationComponent->rotation)
			* glm::scale(glm::vec3(entityRenderData->transformationComponent->scale));

		int rows = 1;
		int columns = 1;
		glm::vec2 textureOffset;
		TextureAtlasIndexComponent *textureAtlasComponent = entityRenderData->textureAtlasIndexComponent;
		if (textureAtlasComponent && contains(textureAtlasComponent->meshToIndexMap, currentMesh))
		{
			rows = textureAtlasComponent->rows;
			columns = textureAtlasComponent->columns;
			int texPos = textureAtlasComponent->meshToIndexMap[currentMesh];
			int col = texPos % columns;
			int row = texPos / columns;
			textureOffset = glm::vec2((float)col / columns, (float)row / rows);
		}

		glm::mat4 mvpTransformation = _renderData.viewProjectionMatrix * modelMatrix;
		glm::mat4 prevTransformation = glm::mix(_renderData.viewProjectionMatrix, _renderData.prevViewProjectionMatrix, 0.15f) * entityRenderData->transformationComponent->prevTransformation;

		uAtlasDataG.set(glm::vec4(columns, rows, textureOffset));
		uModelMatrixG.set(modelMatrix);
		uModelViewProjectionMatrixG.set(mvpTransformation);
		uPrevTransformG.set(prevTransformation);
		uVelG.set(entityRenderData->transformationComponent->vel);
		uExposureTimeG.set(0.001f /* Engine::getCurrentFps()*/ * 60.0f);

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

void SceneRenderer::renderSkybox(const RenderData &_renderData, const std::shared_ptr<Level> &_level)
{
	static const glm::vec4 DEFAULT_ALBEDO_COLOR(1.0);
	static glm::mat4 prevTransform;

	fullscreenTriangle->enableVertexAttribArrays();
	skyboxShader->bind();

	EntityManager &entityManager = EntityManager::getInstance();

	uAlbedoMapB.set(0);
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

	fullscreenTriangle->render();
	//glDrawArrays(GL_TRIANGLES, 0, 3);
}

void SceneRenderer::renderEnvironmentLight(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const glm::mat4 &_inverseView, const glm::mat4 &_inverseProjection, const Effects &_effects)
{
	fullscreenTriangle->enableVertexAttribArrays();

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, ssaoTextureB);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _level->environment.environmentProbe->getIrradianceMap()->getId());
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _level->environment.environmentProbe->getReflectanceMap()->getId());
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, _level->environment.brdfMap->getId());
	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, gLightColorTextures[(currentLightColorTexture + 1) % 2]);

	environmentLightPassShader->bind();

	uViewE.set(_renderData.viewMatrix);
	uInverseViewE.set(_inverseView);
	uProjectionE.set(_renderData.projectionMatrix);
	uInverseProjectionE.set(_inverseProjection);
	uCamPosE.set(_renderData.cameraPosition);
	uAlbedoMapE.set(0);
	uNormalMapE.set(1);
	uMetallicRoughnessAoMapE.set(2);
	uDepthMapE.set(3);
	uSsaoMapE.set(4);
	uIrradianceMapE.set(6);
	uPrefilterMapE.set(7);
	uBrdfLUTE.set(8);
	uPrevFrameE.set(9);
	uSsaoE.set(_effects.ssao.enabled);
	uUseSsrE.set(_effects.screenSpaceReflections.enabled);

	static glm::mat4 prevViewProjection;

	uPrevViewProjectionE.set(_renderData.viewProjectionMatrix);
	prevViewProjection = _renderData.viewProjectionMatrix;

	if (!_level->lights.directionalLights.empty())
	{
		std::shared_ptr<DirectionalLight> directionalLight = _level->lights.directionalLights[0];
		if (directionalLight->isRenderShadows())
		{
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, directionalLight->getShadowMap());
		}
		uDirectionalLightE.set(directionalLight, 5);
		uRenderDirectionalLightE.set(true);
		uShadowsEnabledE.set(_renderData.shadows);
	}
	else
	{
		uRenderDirectionalLightE.set(false);
	}

	fullscreenTriangle->render();
	//glDrawArrays(GL_TRIANGLES, 0, 3);
}

void SceneRenderer::renderDirectionalLights(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const glm::mat4 &_inverseView, const glm::mat4 &_inverseProjection)
{
	fullscreenTriangle->enableVertexAttribArrays();

	directionalLightShader->bind();

	uInverseViewD.set(_inverseView);
	uInverseProjectionD.set(_inverseProjection);
	uCamPosD.set(_renderData.cameraPosition);
	uAlbedoMapD.set(0);
	uNormalMapD.set(1);
	uMetallicRoughnessAoMapD.set(2);
	uDepthMapD.set(3);
	uShadowsEnabledD.set(_renderData.shadows);

	for (size_t i = _level->environment.skyboxEntity ? 1 : 0; i < _level->lights.directionalLights.size(); ++i)
	{
		std::shared_ptr<DirectionalLight> directionalLight = _level->lights.directionalLights[i];
		if (directionalLight->isRenderShadows())
		{
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, directionalLight->getShadowMap());
		}

		uDirectionalLightD.set(directionalLight, 4);
		fullscreenTriangle->render();
		//glDrawArrays(GL_TRIANGLES, 0, 3);
	}
}

void SceneRenderer::renderPointLights(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const glm::mat4 &_inverseView, const glm::mat4 &_inverseProjection)
{
	pointLightMesh->enableVertexAttribArrays();

	pointLightPassShader->bind();

	uAlbedoMapP.set(0);
	uNormalMapP.set(1);
	uMetallicRoughnessAoMapP.set(2);
	uDepthMapP.set(3);
	uInverseViewP.set(_inverseView);
	uInverseProjectionP.set(_inverseProjection);
	uCamPosP.set(_renderData.cameraPosition);
	uShadowsEnabledP.set(_renderData.shadows);
	uViewportSizeP.set(glm::vec2(_renderData.resolution.first, _renderData.resolution.second));

	for (std::shared_ptr<PointLight> pointLight : _level->lights.pointLights)
	{
		if (pointLight->isRenderShadows())
		{
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, pointLight->getShadowMap());
		}

		uModelViewProjectionP.set(_renderData.viewProjectionMatrix * glm::translate(pointLight->getPosition()) * glm::scale(glm::vec3(calculateLightScale(pointLight->getColor()))));
		uPointLightP.set(pointLight, 4);
		pointLightMesh->render();
	}
}

void SceneRenderer::renderSpotLights(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const glm::mat4 &_inverseView, const glm::mat4 &_inverseProjection)
{
	spotLightMesh->enableVertexAttribArrays();

	spotLightPassShader->bind();

	uAlbedoMapS.set(0);
	uNormalMapS.set(1);
	uMetallicRoughnessAoMapS.set(2);
	uDepthMapS.set(3);
	uInverseViewS.set(_inverseView);
	uInverseProjectionS.set(_inverseProjection);
	uCamPosS.set(_renderData.cameraPosition);
	uShadowsEnabledS.set(_renderData.shadows);
	uViewportSizeS.set(glm::vec2(_renderData.resolution.first, _renderData.resolution.second));

	for (std::shared_ptr<SpotLight> spotLight : _level->lights.spotLights)
	{
		if (spotLight->isRenderShadows())
		{
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, spotLight->getShadowMap());
		}

		glm::vec3 position = spotLight->getPosition();
		glm::vec3 direction = spotLight->getDirection();
		glm::vec3 up = (abs(direction.x) < 0.0001 && abs(direction.z) < 0.0001) ? glm::vec3(0.0f, 1.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
		glm::mat4 lightView = glm::lookAt(position, position + direction, up);

		float a = glm::angle(glm::vec3(0.0f, 0.0f, 1.0f), direction);
		float ad = glm::degrees(a);
		glm::mat4 rot = glm::rotate(a, glm::vec3(0.0f, 1.0f, 0.0f));
		a = glm::angle(glm::vec3(0.0f, 1.0f, 0.0f), direction);
		ad = glm::degrees(a);
		rot *= glm::rotate(a, glm::vec3(1.0f, 0.0f, 0.0f));
		//rot *= glm::rotate(glm::angle(glm::vec3(0.0f, 0.0f, 1.0f), direction), glm::vec3(0.0f, 0.0f, 1.0f));

		uModelViewProjectionS.set(_renderData.viewProjectionMatrix * glm::translate(position) * rot * glm::scale(glm::vec3(calculateLightScale(spotLight->getColor()))));
		uSpotLightS.set(spotLight, 4);
		spotLightMesh->render();
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
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, _level->lights.directionalLights[0]->getShadowMap());
		}
		uRenderDirectionalLightT.set(true);
		uDirectionalLightT.set(_level->lights.directionalLights[0], 4);
	}
	else
	{
		uRenderDirectionalLightT.set(false);
	}
	uShadowsEnabledT.set(_renderData.shadows);
	uIrradianceMapT.set(6);
	uPrefilterMapT.set(7);
	uBrdfLUTT.set(8);
	uCamPosT.set(_renderData.cameraPosition);

	const std::vector<std::unique_ptr<EntityRenderData>> &data = _scene.getData();

	std::shared_ptr<Mesh> currentMesh = nullptr;
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
		if (textureAtlasComponent && contains(textureAtlasComponent->meshToIndexMap, currentMesh))
		{
			rows = textureAtlasComponent->rows;
			columns = textureAtlasComponent->columns;
			int texPos = textureAtlasComponent->meshToIndexMap[currentMesh];
			int col = texPos % columns;
			int row = texPos / columns;
			textureOffset = glm::vec2((float)col / columns, (float)row / rows);
		}

		glm::mat4 mvpTransform = _renderData.viewProjectionMatrix * modelMatrix;
		glm::mat4 prevTransformation = glm::mix(_renderData.viewProjectionMatrix, _renderData.prevViewProjectionMatrix, 0.15f) * entityRenderData->transformationComponent->prevTransformation;

		uAtlasDataT.set(glm::vec4(columns, rows, textureOffset));
		uModelMatrixT.set(modelMatrix);
		uModelViewProjectionMatrixT.set(mvpTransform);
		uPrevTransformT.set(prevTransformation);

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

	std::shared_ptr<Mesh> currentMesh = nullptr;
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

	std::shared_ptr<Mesh> currentMesh = nullptr;
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

void SceneRenderer::renderSsaoTexture(const RenderData &_renderData, const glm::mat4 &_inverseProjection, const Effects &_effects)
{
	static bool original = false;

	if (original)
	{
		fullscreenTriangle->enableVertexAttribArrays();
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFbo);
		glViewport(0, 0, _renderData.resolution.first / 2, _renderData.resolution.second / 2);

		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);

		ssaoOriginalShader->bind();
		uDepthTextureAOO.set(3);
		uNoiseTextureAOO.set(5);

		fullscreenTriangle->render();

		glDrawBuffer(GL_COLOR_ATTACHMENT1);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, ssaoTextureA);

		ssaoBlurShader->bind();
		uInputTextureAOB.set(6);
		uBlurSizeAOB.set(4);

		fullscreenTriangle->render();
	}
	else
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
			generateKernel = false;
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

		fullscreenTriangle->enableVertexAttribArrays();

		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFbo);
		glViewport(0, 0, _renderData.resolution.first / 2, _renderData.resolution.second / 2);

		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);

		ssaoShader->bind();
		uDepthTextureAO.set(3);
		uNormalTextureAO.set(1);
		uNoiseTextureAO.set(5);
		uViewAO.set(_renderData.viewMatrix);
		uProjectionAO.set(_renderData.projectionMatrix);
		uInverseProjectionAO.set(_inverseProjection);
		uKernelSizeAO.set((int)currentKernelSize);
		uRadiusAO.set(_effects.ssao.radius);
		uBiasAO.set(_effects.ssao.bias);
		for (unsigned int i = 0; i < currentKernelSize; ++i)
		{
			ssaoShader->setUniform(uSamplesAO[i], ssaoKernel[i]);
		}

		fullscreenTriangle->render();
		//glDrawArrays(GL_TRIANGLES, 0, 3);

		glDrawBuffer(GL_COLOR_ATTACHMENT1);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, ssaoTextureA);

		ssaoBlurShader->bind();
		uInputTextureAOB.set(6);
		uBlurSizeAOB.set(4);

		fullscreenTriangle->render();
		//glDrawArrays(GL_TRIANGLES, 0, 3);
	}

void SceneRenderer::precomputeFftTextures()
{
	fullscreenTriangle->enableVertexAttribArrays();

	// tildeh0k/minusk
	{
		tildeH0kShader->bind();
		uNoiseR0TextureH0.set(0);
		uNoiseI0TextureH0.set(1);
		uNoiseR1TextureH0.set(2);
		uNoiseI1TextureH0.set(3);

		uNH0.set(N);
		uLH0.set(L);
		uAH0.set(A);
		uWindDirectionH0.set(windDirection);
		uWindSpeedH0.set(windSpeed);
		uWaveSuppressionExpH0.set(6.0f);

		std::shared_ptr<Texture> noise0 = Texture::createTexture("Resources/Textures/Noise256_0.dds", true);
		std::shared_ptr<Texture> noise1 = Texture::createTexture("Resources/Textures/Noise256_1.dds", true);
		std::shared_ptr<Texture> noise2 = Texture::createTexture("Resources/Textures/Noise256_2.dds", true);
		std::shared_ptr<Texture> noise3 = Texture::createTexture("Resources/Textures/Noise256_3.dds", true);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, noise0->getId());
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, noise1->getId());
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, noise2->getId());
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, noise3->getId());

		glBindFramebuffer(GL_FRAMEBUFFER, fftFbo);
		glViewport(0, 0, N, N);

		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, drawBuffers);

		fullscreenTriangle->render();
		//glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	// butterfly precompute
	{
		std::uint32_t bitReversedIndices[N];

		for (std::uint32_t i = 0; i < N; ++i)
		{
			std::uint32_t x = glm::bitfieldReverse(i);
			x = glm::bitfieldRotateRight(x, log2N);
			bitReversedIndices[i] = x;
		}

		butterflyPrecomputeShader->bind();
		uNBP.set(N);
		for (unsigned int i = 0; i < N; ++i)
		{
			butterflyPrecomputeShader->setUniform(uJBP[i], (int)bitReversedIndices[i]);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, twiddleIndicesFbo);
		glViewport(0, 0, log2N, N);

		fullscreenTriangle->render();
		//glDrawArrays(GL_TRIANGLES, 0, 3);
	}
}

void SceneRenderer::computeFft()
{
	fullscreenTriangle->enableVertexAttribArrays();
	// tildehkt
	{
		tildeHktShader->bind();
		uTildeH0kTextureHT.set(0);
		uTildeH0minusKTextureHT.set(1);
		uNHT.set(N);
		uLHT.set(L);
		uTimeHT.set((float)Engine::getCurrentTime() * 1.25f);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tildeH0kTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, tildeH0minusKTexture);

		glBindFramebuffer(GL_FRAMEBUFFER, fftFbo);
		glViewport(0, 0, N, N);

		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
		glDrawBuffers(3, drawBuffers);

		fullscreenTriangle->render();
		//glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	// butterfly computation/ inversion
	{
		butterflyComputeShader->bind();
		uButterflyTextureBC.set(0);
		uInputXTextureBC.set(1);
		uInputYTextureBC.set(2);
		uInputZTextureBC.set(3);
		uNBC.set(N);
		uStagesBC.set(log2N);

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

			for (int j = 0; j < log2N; ++j)
			{
				uStageBC.set(j);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, inputTextures[drawBuffer][0]);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, inputTextures[drawBuffer][1]);
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, inputTextures[drawBuffer][2]);
				glDrawBuffers(3, drawBuffers[drawBuffer]);

				fullscreenTriangle->render();
				//glDrawArrays(GL_TRIANGLES, 0, 3);
				drawBuffer = 1 - drawBuffer;
			}
		}

		// inverse/permute
		{
			glBindFramebuffer(GL_FRAMEBUFFER, waterFbo);
			glViewport(0, 0, N, N);

			inversePermuteShader->bind();
			uInputXTextureIP.set(0);
			uInputYTextureIP.set(1);
			uInputZTextureIP.set(2);
			uNIP.set(N);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, inputTextures[drawBuffer][0]);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, inputTextures[drawBuffer][1]);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, inputTextures[drawBuffer][2]);

			glDrawBuffer(GL_COLOR_ATTACHMENT0);

			fullscreenTriangle->render();
			//glDrawArrays(GL_TRIANGLES, 0, 3);

			// generate mips
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, waterDisplacementFoldingTexture);
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		// normal
		{
			waterNormalShader->bind();
			uDisplacementTextureN.set(0);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, waterDisplacementFoldingTexture);

			glDrawBuffer(GL_COLOR_ATTACHMENT1);

			fullscreenTriangle->render();
			//glDrawArrays(GL_TRIANGLES, 0, 3);

			// generate mips
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, waterNormalTexture);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
	}
}

void SceneRenderer::renderWater(const RenderData &_renderData, const std::shared_ptr<Level> &_level)
{
	static std::shared_ptr<Texture> foamTexture = Texture::createTexture("Resources/Textures/foam.dds", true);

	waterShader->bind();
	uNormalTextureW.set(0);
	uDisplacementTextureW.set(1);
	uFoamTextureW.set(2);
	uEnvironmentTextureW.set(3);
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

	glBindVertexArray(waterVAO);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElements(GL_TRIANGLES, (GLsizei)(waterGridDimensions.x * waterGridDimensions.y * 6), GL_UNSIGNED_INT, 0);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void SceneRenderer::createWaterPlane(const glm::vec2 &_dimensions, GLuint &_VBO, GLuint &_VAO, GLuint &_EBO)
{
	std::vector<glm::vec2> vertices(std::size_t((_dimensions.x + 1) * ((_dimensions.y + 1))));
	for (std::size_t i = 0, y = 0; y <= _dimensions.y; ++y)
	{
		for (std::size_t x = 0; x <= _dimensions.x; ++x, ++i)
		{
			glm::vec2 pos(x, y);
			pos /= _dimensions;
			pos = pos * 2.0 - glm::vec2(1.0);
			vertices[i] = pos;
		}
	}

	std::vector<GLuint> indices(std::size_t(_dimensions.x * _dimensions.y * 6));
	for (std::size_t ti = 0, vi = 0, y = 0; y < _dimensions.y; ++y, ++vi)
	{
		for (int x = 0; x < _dimensions.x; ++x, ti += 6, ++vi)
		{
			indices[ti] = GLuint(vi);
			indices[ti + 3] = indices[ti + 2] = GLuint(vi + 1);
			indices[ti + 4] = indices[ti + 1] = GLuint(vi + _dimensions.x + 1);
			indices[ti + 5] = GLuint(vi + _dimensions.x + 2);
		}
	}

	glGenVertexArrays(1, &_VAO);
	glGenBuffers(1, &_VBO);
	glGenBuffers(1, &_EBO);

	glBindVertexArray(_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, _VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
}

#ifdef SPHERES
std::shared_ptr<Mesh> mesh;
void renderSphere()
{
	if (!mesh)
	{
		std::vector<glm::vec3> positions;
		std::vector<glm::vec2> uv;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec3> tangents;
		std::vector<glm::vec3> bitangents;
		std::vector<std::uint32_t> indices;

		const unsigned int X_SEGMENTS = 64;
		const unsigned int Y_SEGMENTS = 64;
		const float PI = 3.14159265359f;
		for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
		{
			for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
			{
				float xSegment = (float)x / (float)X_SEGMENTS;
				float ySegment = (float)y / (float)Y_SEGMENTS;
				float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
				float yPos = std::cos(ySegment * PI);
				float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

				positions.push_back(glm::vec3(xPos, yPos, zPos));
				uv.push_back(glm::vec2(xSegment, ySegment));
				glm::vec3 normal = glm::vec3(xPos, yPos, zPos);
				normals.push_back(normal);

				xPos = std::cos(xSegment + X_SEGMENTS / 4 * 2.0f * PI) * std::sin(ySegment * PI);
				yPos = std::cos(ySegment * PI);
				zPos = std::sin(xSegment + X_SEGMENTS / 4 * 2.0f * PI) * std::sin(ySegment * PI);
				glm::vec3 tangent = glm::vec3(xPos, yPos, zPos);
				tangents.push_back(tangent);
				bitangents.push_back(glm::cross(tangent, normal));
			}
		}


		for (int y = 0; y < Y_SEGMENTS; ++y)
		{

			for (int x = 0; x <= X_SEGMENTS; ++x)
			{
				indices.push_back(y       * (X_SEGMENTS + 1) + x);
				indices.push_back((y + 1) * (X_SEGMENTS + 1) + x + 1);
				indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);

				indices.push_back(y       * (X_SEGMENTS + 1) + x);
				indices.push_back(y       * (X_SEGMENTS + 1) + x + 1);
				indices.push_back((y + 1) * (X_SEGMENTS + 1) + x + 1);
			}

		}

		std::vector<Vertex> vertices;
		for (size_t i = 0; i < positions.size(); ++i)
		{
			Vertex vertex;
			vertex.position = positions[i];
			vertex.normal = normals[i];
			vertex.texCoords = uv[i];
			vertex.tangent = tangents[i];
			vertex.bitangent = bitangents[i];
			vertices.push_back(vertex);
		}

		mesh = Mesh::createMesh(vertices, indices);

	}

	mesh->enableVertexAttribArrays();
	mesh->render();
}
#endif // SPHERES
