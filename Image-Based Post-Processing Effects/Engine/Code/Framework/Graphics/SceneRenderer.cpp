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
#include ".\..\..\Graphics\Terrain\TileRing.h"
#include "MediumDescription.h"

//#define SPHERES

#ifdef SPHERES
void renderSphere();
#endif // SPHERES

MediumDescription mediumDescrp;

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

	GLuint textures[] = { gAlbedoTexture, gNormalTexture, gMRASTexture, gDepthStencilTexture, gLightColorTextures[0], gLightColorTextures[1],
		gVelocityTexture, brdfLUT, tildeH0kTexture, tildeH0minusKTexture, tildeHktDxTexture, tildeHktDyTexture, tildeHktDzTexture, pingPongTextureA, pingPongTextureB, pingPongTextureC,
		twiddleIndicesTexture, waterDisplacementFoldingTexture, waterNormalTexture };
	glDeleteTextures(sizeof(textures) / sizeof(GLuint), textures);

	GLuint fbos[] = { gBufferFBO, ssaoFbo, fftFbo, twiddleIndicesFbo, waterFbo };

	glDeleteFramebuffers(sizeof(fbos) / sizeof(GLuint), fbos);

	for (int i = 0; i < 6; ++i)
	{
		delete tileRings[i];
	}
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
	hbaoShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Renderer/hbao.frag");
	tildeH0kShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Water/tildeH0k.frag");
	tildeHktShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Water/tildeHkt.frag");
	butterflyPrecomputeShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Water/butterflyPrecompute.frag");
	butterflyComputeShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Water/butterflyCompute.frag");
	inversePermuteShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Water/inversePermute.frag");
	waterNormalShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Water/normal.frag");
	waterShader = ShaderProgram::createShaderProgram("Resources/Shaders/Water/water.vert", "Resources/Shaders/Water/Water.frag");
	waterTessShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/terrain.vert", "Resources/Shaders/Water/water.frag", "Resources/Shaders/Shared/terrain.tessc", "Resources/Shaders/Shared/terrain.tesse");
	lightVolumeShader = ShaderProgram::createShaderProgram("Resources/Shaders/Renderer/lightVolume.vert", "Resources/Shaders/Renderer/lightVolume.frag", "Resources/Shaders/Renderer/lightVolume.tessc", "Resources/Shaders/Renderer/lightVolume.tesse");
	phaseLUTShader = ShaderProgram::createShaderProgram("Resources/Shaders/Renderer/phaseLookup.comp");

	tildeH0kCompShader = ShaderProgram::createShaderProgram("Resources/Shaders/Water/tildeH0k.comp");
	tildeHktCompShader = ShaderProgram::createShaderProgram("Resources/Shaders/Water/tildeHkt.comp");
	butterflyPrecomputeCompShader = ShaderProgram::createShaderProgram("Resources/Shaders/Water/butterflyPrecompute.comp");
	butterflyComputeCompShader = ShaderProgram::createShaderProgram("Resources/Shaders/Water/butterflyCompute.comp");
	inversePermuteCompShader = ShaderProgram::createShaderProgram("Resources/Shaders/Water/inversePermute.comp");
	waterNormalCompShader = ShaderProgram::createShaderProgram("Resources/Shaders/Water/normal.comp");

	// create uniforms

	// gBufferPass uniforms
	uMaterialG.create(gBufferPassShader);
	uModelViewMatrixG.create(gBufferPassShader);
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
	uIrradianceMapE.create(environmentLightPassShader);
	uPrefilterMapE.create(environmentLightPassShader);
	uBrdfLUTE.create(environmentLightPassShader);
	uDirectionalLightE.create(environmentLightPassShader);
	uShadowsEnabledE.create(environmentLightPassShader);
	uRenderDirectionalLightE.create(environmentLightPassShader);
	uSsaoE.create(environmentLightPassShader);
	uPrevFrameE.create(environmentLightPassShader);
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
	uStrengthAO.create(ssaoShader);

	// ssao original
	uDepthTextureAOO.create(ssaoOriginalShader);
	uNoiseTextureAOO.create(ssaoOriginalShader);

	// ssao blur
	uInputTextureAOB.create(ssaoBlurShader);
	uBlurSizeAOB.create(ssaoBlurShader);

	// hbao
	uDepthMapHBAO.create(hbaoShader);
	uNoiseMapHBAO.create(hbaoShader);
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

	// tildeh0k
	uNoiseR0TextureH0.create(tildeH0kShader);
	uNoiseI0TextureH0.create(tildeH0kShader);
	uNoiseR1TextureH0.create(tildeH0kShader);
	uNoiseI1TextureH0.create(tildeH0kShader);
	uSimulationResolutionH0.create(tildeH0kShader);
	uWorldSizeH0.create(tildeH0kShader);
	uWaveAmplitudeH0.create(tildeH0kShader);
	uWindDirectionH0.create(tildeH0kShader);
	uWindSpeedH0.create(tildeH0kShader);

	// tildehkt
	uTildeH0kTextureHT.create(tildeHktShader);
	uTildeH0minusKTextureHT.create(tildeHktShader);
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
	uButterflyTextureBC.create(butterflyComputeShader);
	uInputXTextureBC.create(butterflyComputeShader);
	uInputYTextureBC.create(butterflyComputeShader);
	uInputZTextureBC.create(butterflyComputeShader);
	uSimulationResolutionBC.create(butterflyComputeShader);
	uStageBC.create(butterflyComputeShader);
	uStagesBC.create(butterflyComputeShader);
	uDirectionBC.create(butterflyComputeShader);

	// inverse / permute
	uInputXTextureIP.create(inversePermuteShader);
	uInputYTextureIP.create(inversePermuteShader);
	uInputZTextureIP.create(inversePermuteShader);
	uSimulationResolutionIP.create(inversePermuteShader);
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

	// tildeh0k compute
	uNoiseR0TextureH0C.create(tildeH0kCompShader);
	uNoiseI0TextureH0C.create(tildeH0kCompShader);
	uNoiseR1TextureH0C.create(tildeH0kCompShader);
	uNoiseI1TextureH0C.create(tildeH0kCompShader);
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

	// water tesselated
	uNormalTextureWT.create(waterTessShader);
	uDisplacementTextureWT.create(waterTessShader);
	uFoamTextureWT.create(waterTessShader);
	uEnvironmentTextureWT.create(waterTessShader);
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

	// create FBO
	glGenFramebuffers(1, &gBufferFBO);
	glGenFramebuffers(1, &fftFbo);
	glGenFramebuffers(1, &twiddleIndicesFbo);
	glGenFramebuffers(1, &waterFbo);
	glGenFramebuffers(1, &ssaoFbo);

	auto res = std::make_pair(window->getWidth(), window->getHeight());
	createFboAttachments(res);
	createSsaoAttachments(res);

	pointLightMesh = Mesh::createMesh("Resources/Models/pointlight.mesh", 1, true);
	spotLightMesh = Mesh::createMesh("Resources/Models/spotlight.mesh", 1, true);
	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);

	createWaterPlane(waterGridDimensions, waterVBO, waterVAO, waterEBO);
	createLightVolumeMesh(64, lightVolumeVBO, lightVolumeVAO, lightVolumeEBO);
	createBrdfLUT();
	computePhaseLUT();
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

	// render ordinary geometry
	renderGeometry(_renderData, _scene);

	// render custom opaque geometry
	renderCustomGeometry(_renderData, _level, _scene, true);

	// disable stencil testing for now; we will continue writing to the stencil when we do transparency
	glDisable(GL_STENCIL_TEST);

	// calculate matrix inputs for lighting shaders
	glm::mat4 invView = glm::inverse(_renderData.viewMatrix);
	glm::mat4 invProj = glm::inverse(_renderData.projectionMatrix);

	// ocean
	if (_level->water.enabled)
	{

		// compute water textures
		static Water currentWaterConfig = {};

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
		renderSsaoTexture(_renderData, invProj, _effects);
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

	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);
	renderLightVolume(_renderData, _level);
	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);

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

	glGenTextures(1, &lightVolumeTexture);
	glBindTexture(GL_TEXTURE_2D, lightVolumeTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, _resolution.first, _resolution.second, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, lightVolumeTexture, 0);

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

void SceneRenderer::createWaterAttachments(unsigned int _resolution)
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, _resolution.first, _resolution.second, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoTextureA, 0);

	glGenTextures(1, &ssaoTextureB);
	glBindTexture(GL_TEXTURE_2D, ssaoTextureB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, _resolution.first, _resolution.second, 0, GL_RED, GL_FLOAT, NULL);
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
		if (entityRenderData->transparencyComponent && contains(entityRenderData->transparencyComponent->transparentSubMeshes, currentMesh))
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

		if (cullAABB(mvpTransformation, currentMesh->getAABB()))
		{
			continue;
		}

		uAtlasDataG.set(glm::vec4(columns, rows, textureOffset));
		uModelViewMatrixG.set(glm::mat3(_renderData.viewMatrix * modelMatrix));
		uModelViewProjectionMatrixG.set(mvpTransformation);
		uPrevTransformG.set(prevTransformation);
		uVelG.set(entityRenderData->transformationComponent->vel / glm::vec2(_renderData.resolution.first, _renderData.resolution.second));
		uExposureTimeG.set((float(Engine::getCurrentFps()) / 60.0f));

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

	fullscreenTriangle->getSubMesh()->render();
	//glDrawArrays(GL_TRIANGLES, 0, 3);
}

void SceneRenderer::renderEnvironmentLight(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const glm::mat4 &_inverseView, const glm::mat4 &_inverseProjection, const Effects &_effects)
{
	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, ssaoTextureB);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _level->environment.environmentProbe->getIrradianceMap()->getId());
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _level->environment.environmentProbe->getReflectanceMap()->getId());
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, brdfLUT);
	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, gLightColorTextures[(currentLightColorTexture + 1) % 2]);

	environmentLightPassShader->bind();

	uInverseViewE.set(_inverseView);
	uProjectionE.set(_renderData.projectionMatrix);
	uInverseProjectionE.set(_inverseProjection);
	uAlbedoMapE.set(0);
	uNormalMapE.set(1);
	uMetallicRoughnessAoMapE.set(2);
	uDepthMapE.set(3);
	uSsaoMapE.set(4);
	uIrradianceMapE.set(6);
	uPrefilterMapE.set(7);
	uBrdfLUTE.set(8);
	uPrevFrameE.set(9);
	uSsaoE.set(_effects.ambientOcclusion != AmbientOcclusion::OFF);
	uUseSsrE.set(_effects.screenSpaceReflections.enabled);

	static glm::mat4 prevViewProjection;

	uPrevViewProjectionE.set(_renderData.viewProjectionMatrix);
	prevViewProjection = _renderData.viewProjectionMatrix;

	if (!_level->lights.directionalLights.empty())
	{
		std::shared_ptr<DirectionalLight> directionalLight = _level->lights.directionalLights[0];
		directionalLight->updateViewValues(_renderData.viewMatrix);
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

	fullscreenTriangle->getSubMesh()->render();
	//glDrawArrays(GL_TRIANGLES, 0, 3);
}

void SceneRenderer::renderDirectionalLights(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const glm::mat4 &_inverseView, const glm::mat4 &_inverseProjection)
{
	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	directionalLightShader->bind();

	uInverseViewD.set(_inverseView);
	uInverseProjectionD.set(_inverseProjection);
	uAlbedoMapD.set(0);
	uNormalMapD.set(1);
	uMetallicRoughnessAoMapD.set(2);
	uDepthMapD.set(3);
	uShadowsEnabledD.set(_renderData.shadows);

	for (size_t i = _level->environment.skyboxEntity ? 1 : 0; i < _level->lights.directionalLights.size(); ++i)
	{
		std::shared_ptr<DirectionalLight> directionalLight = _level->lights.directionalLights[i];
		directionalLight->updateViewValues(_renderData.viewMatrix);
		if (directionalLight->isRenderShadows())
		{
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, directionalLight->getShadowMap());
		}

		uDirectionalLightD.set(directionalLight, 4);
		fullscreenTriangle->getSubMesh()->render();
		//glDrawArrays(GL_TRIANGLES, 0, 3);
	}
}

void SceneRenderer::renderPointLights(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const glm::mat4 &_inverseView, const glm::mat4 &_inverseProjection)
{
	pointLightMesh->getSubMesh()->enableVertexAttribArrays();

	pointLightPassShader->bind();

	uAlbedoMapP.set(0);
	uNormalMapP.set(1);
	uMetallicRoughnessAoMapP.set(2);
	uDepthMapP.set(3);
	uInverseViewP.set(_inverseView);
	uInverseProjectionP.set(_inverseProjection);
	uShadowsEnabledP.set(_renderData.shadows);
	uViewportSizeP.set(glm::vec2(_renderData.resolution.first, _renderData.resolution.second));

	for (std::shared_ptr<PointLight> pointLight : _level->lights.pointLights)
	{
		pointLight->updateViewValues(_renderData.viewMatrix);

		if (pointLight->isRenderShadows())
		{
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, pointLight->getShadowMap());
		}

		uModelViewProjectionP.set(_renderData.viewProjectionMatrix * glm::translate(pointLight->getPosition()) * glm::scale(glm::vec3(calculateLightScale(pointLight->getColor()))));
		uPointLightP.set(pointLight, 4);
		pointLightMesh->getSubMesh()->render();
	}
}

void SceneRenderer::renderSpotLights(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const glm::mat4 &_inverseView, const glm::mat4 &_inverseProjection)
{
	spotLightMesh->getSubMesh()->enableVertexAttribArrays();

	spotLightPassShader->bind();

	uAlbedoMapS.set(0);
	uNormalMapS.set(1);
	uMetallicRoughnessAoMapS.set(2);
	uDepthMapS.set(3);
	uInverseViewS.set(_inverseView);
	uInverseProjectionS.set(_inverseProjection);
	uShadowsEnabledS.set(_renderData.shadows);
	uViewportSizeS.set(glm::vec2(_renderData.resolution.first, _renderData.resolution.second));

	for (std::shared_ptr<SpotLight> spotLight : _level->lights.spotLights)
	{
		spotLight->updateViewValues(_renderData.viewMatrix);

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
		if (entityRenderData->transparencyComponent && !contains(entityRenderData->transparencyComponent->transparentSubMeshes, currentMesh))
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

void SceneRenderer::renderSsaoTexture(const RenderData &_renderData, const glm::mat4 &_inverseProjection, const Effects &_effects)
{
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
		uDepthTextureAOO.set(3);
		uNoiseTextureAOO.set(5);

		fullscreenTriangle->getSubMesh()->render();

		glDrawBuffer(GL_COLOR_ATTACHMENT1);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, ssaoTextureA);

		ssaoBlurShader->bind();
		uInputTextureAOB.set(6);
		uBlurSizeAOB.set(4);

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
		uDepthTextureAO.set(3);
		uNormalTextureAO.set(1);
		uNoiseTextureAO.set(5);
		uViewAO.set(_renderData.viewMatrix);
		uProjectionAO.set(_renderData.projectionMatrix);
		uInverseProjectionAO.set(_inverseProjection);
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

		glDrawBuffer(GL_COLOR_ATTACHMENT1);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, ssaoTextureA);

		ssaoBlurShader->bind();
		uInputTextureAOB.set(6);
		uBlurSizeAOB.set(4);

		fullscreenTriangle->getSubMesh()->render();
		//glDrawArrays(GL_TRIANGLES, 0, 3);
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
		uDepthMapHBAO.set(3);
		uNoiseMapHBAO.set(5);
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

		glDrawBuffer(GL_COLOR_ATTACHMENT1);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, ssaoTextureA);

		ssaoBlurShader->bind();
		uInputTextureAOB.set(6);
		uBlurSizeAOB.set(4);

		fullscreenTriangle->getSubMesh()->render();

		break;
	}
	default:
		assert(false);
	}
}

static bool waterCompute = true;

void SceneRenderer::precomputeFftTextures(const Water &_water)
{
	std::shared_ptr<Texture> noise0;
	std::shared_ptr<Texture> noise1;
	std::shared_ptr<Texture> noise2;
	std::shared_ptr<Texture> noise3;

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

	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	if (waterCompute)
	{
		// tildeh0k/minusk
		{
			tildeH0kCompShader->bind();
			uNoiseR0TextureH0C.set(0);
			uNoiseI0TextureH0C.set(1);
			uNoiseR1TextureH0C.set(2);
			uNoiseI1TextureH0C.set(3);

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
		// tildeh0k/minusk
		{
			tildeH0kShader->bind();
			uNoiseR0TextureH0.set(0);
			uNoiseI0TextureH0.set(1);
			uNoiseR1TextureH0.set(2);
			uNoiseI1TextureH0.set(3);

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

void SceneRenderer::computeFft(const Water &_water)
{
	if (waterCompute)
	{
		// tildehkt
		{
			tildeHktCompShader->bind();
			uSimulationResolutionHTC.set(_water.simulationResolution);
			uWorldSizeHTC.set(_water.worldSize);
			uTimeHTC.set((float)Engine::getCurrentTime() * _water.timeScale);

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

				for (int j = 0; j < glm::log2(_water.simulationResolution); ++j)
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
			uTildeH0kTextureHT.set(0);
			uTildeH0minusKTextureHT.set(1);
			uSimulationResolutionHT.set(_water.simulationResolution);
			uWorldSizeHT.set(_water.worldSize);
			uTimeHT.set((float)Engine::getCurrentTime() * _water.timeScale);

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
			uButterflyTextureBC.set(0);
			uInputXTextureBC.set(1);
			uInputYTextureBC.set(2);
			uInputZTextureBC.set(3);
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

				for (int j = 0; j < glm::log2(_water.simulationResolution); ++j)
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
				uInputXTextureIP.set(0);
				uInputYTextureIP.set(1);
				uInputZTextureIP.set(2);
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
				uDisplacementTextureN.set(0);

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

void SceneRenderer::renderWater(const RenderData &_renderData, const std::shared_ptr<Level> &_level)
{
	static std::shared_ptr<Texture> foamTexture = Texture::createTexture("Resources/Textures/foam.dds", true);
	static bool useTesselation = true;

	if (useTesselation)
	{
		waterTessShader->bind();
		uNormalTextureWT.set(0);
		uDisplacementTextureWT.set(1);
		uFoamTextureWT.set(2);
		uEnvironmentTextureWT.set(3);
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

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		for (int i = 0; i < 6; ++i)
		{
			uTileSizeWT.set(tileRings[i]->getTileSize());
			tileRings[i]->render();
		}
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else
	{
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

void SceneRenderer::createLightVolumeMesh(unsigned int _size, GLuint & _VBO, GLuint & _VAO, GLuint & _EBO)
{
	GLuint *indices = new GLuint[_size * _size * 4];
	int currentIndex = 0;
	for (int row = 0; row < _size; ++row)
	{
		for (int column = 0; column < _size; ++column)
		{
			indices[currentIndex++] = (row * (_size + 1) + column);
			indices[currentIndex++] = (row * (_size + 1) + column) + 1;
			indices[currentIndex++] = ((row + 1) * (_size + 1) + column) + 1;
			indices[currentIndex++] = ((row + 1) * (_size + 1) + column);
		}
	}

	glm::vec2 *positions = new glm::vec2[(_size + 1) * (_size + 1)];
	for (int y = 0; y < _size + 1; ++y)
	{
		for (int x = 0; x < _size + 1; ++x)
		{
			positions[y * (_size + 1) + x] = glm::vec2(x / float(_size + 1), y / float(_size + 1));
		}
	}

	// create buffers/arrays
	glGenVertexArrays(1, &_VAO);
	glGenBuffers(1, &_VBO);
	glGenBuffers(1, &_EBO);
	glBindVertexArray(_VAO);
	//glBindBuffer(GL_ARRAY_BUFFER, _VBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * (_size + 1) * (_size + 1), positions, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * _size * _size * 4, indices, GL_STATIC_DRAW);

	// patch position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);

	glBindVertexArray(0);

	delete[] indices;
	delete[] positions;
}

GLuint tex;

void SceneRenderer::renderLightVolume(const RenderData & _renderData, const std::shared_ptr<Level>& _level)
{
	const float SCATTER_EPSILON = 0.000001f;
	glm::vec3 total_scatter = glm::vec3(SCATTER_EPSILON, SCATTER_EPSILON, SCATTER_EPSILON);

	for (uint32_t p = 0; p < mediumDescrp.uNumPhaseTerms; ++p)
	{
		total_scatter += mediumDescrp.phaseTerms[p].vDensity;
	}
	glm::vec3 absorption = mediumDescrp.vAbsorption;
	glm::vec3 vScatterPower;
	vScatterPower.x = 1.0 - exp(-total_scatter.x);
	vScatterPower.y = 1.0 - exp(-total_scatter.y);
	vScatterPower.z = 1.0 - exp(-total_scatter.z);
	glm::vec3 vSigmaExtinction = total_scatter + absorption;


	lightVolumeShader->bind();
	uDisplacementTextureLV.set(0);
	uInvLightViewProjectionLV.set(glm::inverse(_level->lights.directionalLights[0]->getViewProjectionMatrix()));
	uViewProjectionLV.set(_renderData.viewProjectionMatrix);

	uPhaseLUTLV.set(1);
	uCamPosLV.set(_renderData.cameraPosition);
	uLightIntensitysLV.set(_level->lights.directionalLights[0]->getColor() * 25000.0);
	uSigmaExtinctionLV.set(vSigmaExtinction);
	uScatterPowerLV.set(vScatterPower);
	uLightDirLV.set(_level->lights.directionalLights[0]->getDirection());
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _level->lights.directionalLights[0]->getShadowMap());

	tex = _level->lights.directionalLights[0]->getShadowMap();

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, phaseLUT);

	glBindVertexArray(lightVolumeVAO);
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawElements(GL_PATCHES, 64 * 64 * 4, GL_UNSIGNED_INT, NULL);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}



void SceneRenderer::computePhaseLUT()
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

	mediumDescrp.phaseTerms[t].ePhaseFunc = 1;
	mediumDescrp.phaseTerms[t].vDensity = (10.00f * SCATTER_PARAM_SCALE * glm::vec3(0.596f, 1.324f, 3.310f));
	t++;

	int mediumType = 0;

	switch (mediumType)
	{
	default:
	case 0:
		mediumDescrp.phaseTerms[t].ePhaseFunc = 2;
		mediumDescrp.phaseTerms[t].vDensity = (10.00f * SCATTER_PARAM_SCALE * glm::vec3(1.00f, 1.00f, 1.00f));
		mediumDescrp.phaseTerms[t].fEccentricity = 0.85f;
		t++;
		mediumDescrp.vAbsorption = (5.0f * SCATTER_PARAM_SCALE * glm::vec3(1, 1, 1));
		break;

	case 1:
		mediumDescrp.phaseTerms[t].ePhaseFunc = 2;
		mediumDescrp.phaseTerms[t].vDensity = (15.00f * SCATTER_PARAM_SCALE * glm::vec3(1.00f, 1.00f, 1.00f));
		mediumDescrp.phaseTerms[t].fEccentricity = 0.60f;
		t++;
		mediumDescrp.vAbsorption = (25.0f * SCATTER_PARAM_SCALE * glm::vec3(1, 1, 1));
		break;

	case 2:
		mediumDescrp.phaseTerms[t].ePhaseFunc = 3;
		mediumDescrp.phaseTerms[t].vDensity = (20.00f * SCATTER_PARAM_SCALE * glm::vec3(1.00f, 1.00f, 1.00f));
		t++;
		mediumDescrp.vAbsorption = (25.0f * SCATTER_PARAM_SCALE * glm::vec3(1, 1, 1));
		break;

	case 3:
		mediumDescrp.phaseTerms[t].ePhaseFunc = 4;
		mediumDescrp.phaseTerms[t].vDensity = (30.00f * SCATTER_PARAM_SCALE * glm::vec3(1.00f, 1.00f, 1.00f));
		t++;
		mediumDescrp.vAbsorption = (50.0f * SCATTER_PARAM_SCALE * glm::vec3(1, 1, 1));
		break;
	}

	uNumPhaseTermsPL.set(t);
	for (int i = 0; i < t; ++i)
	{
		phaseLUTShader->setUniform(uPhaseParamsPL[i], glm::vec4(mediumDescrp.phaseTerms[i].vDensity, mediumDescrp.phaseTerms[i].fEccentricity));
		phaseLUTShader->setUniform(uPhaseFuncPL[i], mediumDescrp.phaseTerms[i].ePhaseFunc);
	}
	
	glBindImageTexture(0, phaseLUT, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	glDispatchCompute(8, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
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
	glDispatchCompute(512, 512, 1);
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

#ifdef SPHERES
std::shared_ptr<SubMesh> mesh;
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

		mesh = SubMesh::createMesh(vertices, indices);

	}

	mesh->enableVertexAttribArrays();
	mesh->render();
}
#endif // SPHERES
