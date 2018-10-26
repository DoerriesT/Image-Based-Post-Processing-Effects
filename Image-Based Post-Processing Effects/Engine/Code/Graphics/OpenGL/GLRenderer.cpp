#include "GLRenderer.h"
#include "RenderData.h"
#include "Level.h"
#include "Graphics/Effects.h"

unsigned int mbTileSize = 40;
unsigned int dofTileSize = 16;

GLRenderer::GLRenderer()
{
}

void GLRenderer::init(unsigned int width, unsigned int height)
{
	m_renderResources.reset(new GLRenderResources(width, height));

	m_shadowRenderPass = std::make_unique<ShadowRenderPass>(m_renderResources->m_shadowFbo, 1, 1); // viewport is reconfigured for every light so the constructor value does not matter
	m_gBufferRenderPass = std::make_unique<GBufferRenderPass>(m_renderResources->m_gBufferFbo, width, height);
	m_gBufferCustomRenderPass = std::make_unique<GBufferCustomRenderPass>(m_renderResources->m_gBufferFbo, width, height);
	m_ssaoOriginalRenderPass = std::make_unique<SSAOOriginalRenderPass>(m_renderResources->m_ssaoFbo, width, height);
	m_ssaoRenderPass = std::make_unique<SSAORenderPass>(m_renderResources->m_ssaoFbo, width, height);
	m_hbaoRenderPass = std::make_unique<HBAORenderPass>(m_renderResources->m_ssaoFbo, width, height);
	m_gtaoRenderPass = std::make_unique<GTAORenderPass>(m_renderResources->m_ssaoFbo, width, height);
	m_gtaoSpatialDenoiseRenderPass = std::make_unique<GTAOSpatialDenoiseRenderPass>(m_renderResources->m_ssaoFbo, width, height);
	m_gtaoTemporalDenoiseRenderPass = std::make_unique<GTAOTemporalDenoiseRenderPass>(m_renderResources->m_ssaoFbo, width, height);
	m_ssaoBlurRenderPass = std::make_unique<SSAOBlurRenderPass>(m_renderResources->m_ssaoFbo, width, height);
	m_ssaoBilateralBlurRenderPass = std::make_unique<SSAOBilateralBlurRenderPass>(m_renderResources->m_ssaoFbo, width, height);
	m_skyboxRenderPass = std::make_unique<SkyboxRenderPass>(m_renderResources->m_gBufferFbo, width, height);
	m_ambientLightRenderPass = std::make_unique<AmbientLightRenderPass>(m_renderResources->m_gBufferFbo, width, height);
	m_directionalLightRenderPass = std::make_unique<DirectionalLightRenderPass>(m_renderResources->m_gBufferFbo, width, height);
	m_stencilRenderPass = std::make_unique<StencilRenderPass>(m_renderResources->m_gBufferFbo, width, height);
	m_deferredEnvironmentProbeRenderPass = std::make_unique<DeferredEnvironmentProbeRenderPass>(m_renderResources->m_gBufferFbo, width, height);
	m_pointLightRenderPass = std::make_unique<PointLightRenderPass>(m_renderResources->m_gBufferFbo, width, height);
	m_spotLightRenderPass = std::make_unique<SpotLightRenderPass>(m_renderResources->m_gBufferFbo, width, height);
	m_forwardRenderPass = std::make_unique<ForwardRenderPass>(m_renderResources->m_gBufferFbo, width, height);
	m_forwardCustomRenderPass = std::make_unique<ForwardCustomRenderPass>(m_renderResources->m_gBufferFbo, width, height);
	m_outlineRenderPass = std::make_unique<OutlineRenderPass>(m_renderResources->m_gBufferFbo, width, height);
	m_lightProbeRenderPass = std::make_unique<LightProbeRenderPass>(m_renderResources->m_gBufferFbo, width, height);

	m_anamorphicPrefilterComputePass = std::make_unique<AnamorphicPrefilterComputePass>(width, height);
	m_anamorphicDownsampleComputePass = std::make_unique<AnamorphicDownsampleComputePass>(width, height);
	m_anamorphicUpsampleComputePass = std::make_unique<AnamorphicUpsampleComputePass>(width, height);
	m_fxaaRenderPass = std::make_unique<FXAARenderPass>(m_renderResources->m_ppFullResolutionFbo, width, height);
	m_smaaEdgeDetectionRenderPass = std::make_unique<SMAAEdgeDetectionRenderPass>(m_renderResources->m_smaaFbo, width, height);
	m_smaaBlendWeightRenderPass = std::make_unique<SMAABlendWeightRenderPass>(m_renderResources->m_smaaFbo, width, height);
	m_smaaBlendRenderPass = std::make_unique<SMAABlendRenderPass>(m_renderResources->m_smaaFbo, width, height);
	m_smaaTemporalResolveRenderPass = std::make_unique<SMAATemporalResolveRenderPass>(m_renderResources->m_smaaFbo, width, height);
	m_godRayMaskComputePass = std::make_unique<GodRayMaskComputePass>(width, height);
	m_godRayGenComputePass = std::make_unique<GodRayGenComputePass>(width, height);
	m_luminanceGenComputePass = std::make_unique<LuminanceGenComputePass>(width, height);
	m_luminanceAdaptionComputePass = std::make_unique<LuminanceAdaptionComputePass>(width, height);
	m_velocityCorrectionComputePass = std::make_unique<VelocityCorrectionComputePass>(width, height);
	m_simpleDofCocBlurComputePass = std::make_unique<SimpleDofCocBlurComputePass>(width, height);
	m_simpleDofBlurComputePass = std::make_unique<SimpleDofBlurComputePass>(width, height);
	m_simpleDofFillComputePass = std::make_unique<SimpleDofFillComputePass>(width, height);
	m_simpleDofCompositeComputePass = std::make_unique<SimpleDofCompositeComputePass>(width, height);
	m_spriteDofRenderPass = std::make_unique<SpriteDofRenderPass>(m_renderResources->m_cocFbo, width, height / 2);
	m_spriteDofCompositeComputePass = std::make_unique<SpriteDofCompositeComputePass>(width, height);
	m_seperateDofDownsampleComputePass = std::make_unique<SeperateDofDownsampleComputePass>(width, height);
	m_seperateDofBlurComputePass = std::make_unique<SeperateDofBlurComputePass>(width, height);
	m_seperateDofFillComputePass = std::make_unique<SeperateDofFillComputePass>(width, height);
	m_seperateDofCompositeComputePass = std::make_unique<SeperateDofCompositeComputePass>(width, height);
	m_luminanceHistogramComputePass = std::make_unique<LuminanceHistogramComputePass>(width, height);
	m_luminanceHistogramReduceComputePass = std::make_unique<LuminanceHistogramReduceComputePass>(width, height);
	m_luminanceHistogramAdaptionComputePass = std::make_unique<LuminanceHistogramAdaptionComputePass>(width, height);
	m_cocComputePass = std::make_unique<CocComputePass>(width, height);
	m_cocTileMaxRenderPass = std::make_unique<CocTileMaxRenderPass>(m_renderResources->m_cocFbo, width, height);
	m_cocNeighborTileMaxRenderPass = std::make_unique<CocNeighborTileMaxRenderPass>(m_renderResources->m_cocFbo, width / dofTileSize, height / dofTileSize);
	m_velocityTileMaxRenderPass = std::make_unique<VelocityTileMaxRenderPass>(m_renderResources->m_velocityFbo, width, height);
	m_velocityNeighborTileMaxRenderPass = std::make_unique<VelocityNeighborTileMaxRenderPass>(m_renderResources->m_velocityFbo, width / mbTileSize, height / mbTileSize);
	m_lensFlareGenRenderPass = std::make_unique<LensFlareGenRenderPass>(m_renderResources->m_ppHalfResolutionFbo, width, height);
	m_lensFlareBlurRenderPass = std::make_unique<LensFlareBlurRenderPass>(m_renderResources->m_ppHalfResolutionFbo, width, height);
	m_bloomDownsampleComputePass = std::make_unique<BloomDownsampleComputePass>(width, height);
	m_bloomUpsampleComputePass = std::make_unique<BloomUpsampleComputePass>(width, height);
	m_simplePostEffectsRenderPass = std::make_unique<SimplePostEffectsRenderPass>(m_renderResources->m_ppFullResolutionFbo, width, height);
	m_toneMapRenderPass = std::make_unique<ToneMapRenderPass>(m_renderResources->m_ppFullResolutionFbo, width, height);
	m_combinedDofTileMaxComputePass = std::make_unique<CombinedDofTileMaxComputePass>(width, height);
	m_combinedDofNeighborTileMaxComputePass = std::make_unique<CombinedDofNeighborTileMaxComputePass>(width, height);
	m_seperateDofTileMaxComputePass = std::make_unique<SeperateDofTileMaxComputePass>(width, height);
	m_antiAliasingTonemapComputePass = std::make_unique<AntiAliasingTonemapComputePass>(width, height);
	m_antiAliasingReverseTonemapComputePass = std::make_unique<AntiAliasingReverseTonemapComputePass>(width, height);

	m_boundingBoxRenderPass = std::make_unique<BoundingBoxRenderPass>(m_renderResources->m_debugFbo, width, height);
}

void GLRenderer::render(const RenderData &renderData, const Scene &scene, const std::shared_ptr<Level> &level, const Effects &effects, bool bake, bool debugDraw)
{
	RenderPass *previousRenderPass = nullptr;
	frame = renderData.frame;

	m_shadowRenderPass->render(renderData, level, scene, true, &previousRenderPass);

	const GLenum lightColorAttachments[] = { GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
	const GLenum firstPassDrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 , lightColorAttachments[frame % 2], GL_COLOR_ATTACHMENT6 };

	// bind g-buffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_renderResources->m_gBufferFbo);
	glDrawBuffers(sizeof(firstPassDrawBuffers) / sizeof(GLenum), firstPassDrawBuffers);

	// enable depth testing and writing and clear all buffers
	glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	m_gBufferRenderPass->render(renderData, scene, &previousRenderPass);
	m_gBufferCustomRenderPass->render(renderData, level, scene, &previousRenderPass);

	// setup all g-buffer textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_renderResources->m_gAlbedoTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_renderResources->m_gNormalTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_renderResources->m_gMRASTexture);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_renderResources->m_gDepthStencilTexture);

	ssaoTexture = m_renderResources->m_ssaoTextureA;
	switch (effects.ambientOcclusion)
	{
	case AmbientOcclusion::SSAO_ORIGINAL:
	{
		m_ssaoOriginalRenderPass->render(renderData, effects, m_renderResources->m_noiseTexture, &previousRenderPass);
		m_ssaoBilateralBlurRenderPass->render(renderData, effects, m_renderResources->m_ssaoTextureA, &previousRenderPass);
		ssaoTexture = m_renderResources->m_ssaoTextureB;
		break;
	}
	case AmbientOcclusion::SSAO:
	{
		m_ssaoRenderPass->render(renderData, effects, m_renderResources->m_noiseTexture, &previousRenderPass);
		m_ssaoBilateralBlurRenderPass->render(renderData, effects, m_renderResources->m_ssaoTextureA, &previousRenderPass);
		ssaoTexture = m_renderResources->m_ssaoTextureB;
		break;
	}
	case AmbientOcclusion::HBAO:
	{
		m_hbaoRenderPass->render(renderData, effects, m_renderResources->m_noiseTexture2, &previousRenderPass);
		m_ssaoBilateralBlurRenderPass->render(renderData, effects, m_renderResources->m_ssaoTextureA, &previousRenderPass);
		ssaoTexture = m_renderResources->m_ssaoTextureB;
		break;
	}
	case AmbientOcclusion::GTAO:
	{
		m_gtaoRenderPass->render(renderData, effects, &previousRenderPass);
		GLuint ssaoTextures[3] = { m_renderResources->m_ssaoTextureA, m_renderResources->m_ssaoTextureB, m_renderResources->m_ssaoTextureC };
		m_gtaoSpatialDenoiseRenderPass->render(renderData, effects, ssaoTextures, &previousRenderPass);
		m_gtaoTemporalDenoiseRenderPass->render(renderData, effects, m_renderResources->m_gVelocityTexture, ssaoTextures, &previousRenderPass);
		ssaoTexture = renderData.frame % 2 ? ssaoTextures[2] : ssaoTextures[0];
		break;
	}
	default:
		break;
	}

	m_skyboxRenderPass->render(renderData, level, &previousRenderPass);
	m_ambientLightRenderPass->render(renderData, level, effects, ssaoTexture, m_renderResources->m_brdfLUT, &previousRenderPass);
	m_directionalLightRenderPass->render(renderData, level, &previousRenderPass);
	m_stencilRenderPass->render(renderData, level, &previousRenderPass);
	m_deferredEnvironmentProbeRenderPass->render(renderData, level, effects, ssaoTexture, m_renderResources->m_brdfLUT, &previousRenderPass);
	m_pointLightRenderPass->render(renderData, level, &previousRenderPass);
	m_spotLightRenderPass->render(renderData, level, &previousRenderPass);
	m_forwardRenderPass->render(renderData, level, scene, &previousRenderPass);
	m_forwardCustomRenderPass->render(renderData, level, scene, &previousRenderPass);
	m_outlineRenderPass->render(renderData, scene, &previousRenderPass);
	m_lightProbeRenderPass->render(renderData, level, &previousRenderPass);

	// generate mips
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_renderResources->m_gLightColorTextures[frame % 2]);
	glGenerateMipmap(GL_TEXTURE_2D);

	if (bake)
	{
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		return;
	}

	GLuint colorTexture = m_renderResources->m_gLightColorTextures[frame % 2];

	// post-processing
	if (effects.smaa.enabled)
	{
		currentSmaaTexture = !currentSmaaTexture;

		m_antiAliasingTonemapComputePass->execute(colorTexture);

		m_smaaEdgeDetectionRenderPass->render(effects, colorTexture, &previousRenderPass);
		m_smaaBlendWeightRenderPass->render(effects, m_renderResources->m_fullResolutionSmaaEdgesTex, effects.smaa.temporalAntiAliasing, currentSmaaTexture, &previousRenderPass);
		m_smaaBlendRenderPass->render(effects, colorTexture, m_renderResources->m_gVelocityTexture, m_renderResources->m_fullResolutionSmaaBlendTex, currentSmaaTexture, &previousRenderPass);

		if (effects.smaa.temporalAntiAliasing)
		{
			m_smaaTemporalResolveRenderPass->render(effects, m_renderResources->m_fullResolutionSmaaMLResultTex, m_renderResources->m_gVelocityTexture, currentSmaaTexture, &previousRenderPass);
			colorTexture = m_renderResources->m_fullResolutionSmaaResultTex;
		}
		else
		{
			colorTexture = m_renderResources->m_fullResolutionSmaaMLResultTex[currentSmaaTexture];
		}

		m_antiAliasingReverseTonemapComputePass->execute(colorTexture);
	}

	// downsample/blur -> upsample/blur/combine with previous result
	// use end result as bloom and input for lens flares
	if (effects.bloom.enabled || effects.lensFlares.enabled)
	{
		m_bloomDownsampleComputePass->execute(colorTexture, m_renderResources->m_halfResolutionHdrTexA);
		m_bloomUpsampleComputePass->execute(m_renderResources->m_halfResolutionHdrTexA, m_renderResources->m_halfResolutionHdrTexB);
	}

	// flares in 1/2 A
	if (effects.lensFlares.enabled)
	{
		// texture B contains combined blurred mipmap chain
		// generate ghosts by sampling blurred texture with threshold
		m_lensFlareGenRenderPass->render(effects, m_renderResources->m_halfResolutionHdrTexB, &previousRenderPass);
		// blur result (maybe skip this step, since source should already be pretty blurry)
		m_lensFlareBlurRenderPass->render(m_renderResources->m_halfResolutionHdrTexA, m_renderResources->m_halfResolutionHdrTexC, &previousRenderPass);
	}

	if (effects.anamorphicFlares.enabled)
	{
		m_anamorphicPrefilterComputePass->execute(effects, colorTexture, m_renderResources->m_anamorphicPrefilter);
		size_t lastUsedTexture;
		unsigned int lastWidth;
		m_anamorphicDownsampleComputePass->execute(effects, m_renderResources->m_anamorphicPrefilter, m_renderResources->m_anamorphicChain, 6, lastUsedTexture, lastWidth);
		m_anamorphicUpsampleComputePass->execute(effects, m_renderResources->m_anamorphicPrefilter, m_renderResources->m_anamorphicChain, 6, lastUsedTexture, lastWidth);
	}

	m_velocityCorrectionComputePass->execute(renderData, m_renderResources->m_gVelocityTexture, m_renderResources->m_gDepthStencilTexture);

	if (effects.motionBlur != MotionBlur::OFF)
	{
		m_velocityTileMaxRenderPass->render(m_renderResources->m_gVelocityTexture, m_renderResources->m_velocityTexTmp, m_renderResources->m_velocityMaxTex, mbTileSize, &previousRenderPass);
		m_velocityNeighborTileMaxRenderPass->render(m_renderResources->m_velocityMaxTex, m_renderResources->m_velocityNeighborMaxTex, &previousRenderPass);
	}

	if (effects.godrays && !level->lights.directionalLights.empty())
	{
		glm::vec2 sunpos = glm::vec2(renderData.viewProjectionMatrix * glm::vec4(level->lights.directionalLights[0]->getDirection(), 0.0f)) * 0.5f + 0.5f;
		GLuint godRayTextures[] = { m_renderResources->m_halfResolutionGodRayTexA, m_renderResources->m_halfResolutionGodRayTexB };
		m_godRayMaskComputePass->execute(effects, colorTexture, m_renderResources->m_gDepthStencilTexture, godRayTextures[0]);
		m_godRayGenComputePass->execute(effects, godRayTextures, sunpos);
	}

	if (effects.depthOfField != DepthOfField::OFF)
	{
		m_cocComputePass->execute(m_renderResources->m_gDepthStencilTexture, m_renderResources->m_fullResolutionCocTexture, glm::radians(renderData.fov), renderData.nearPlane, renderData.farPlane);
	}

	switch (effects.depthOfField)
	{
	case DepthOfField::OFF:
		break;
	case DepthOfField::SIMPLE:
	{
		GLuint cocTextures[] = { m_renderResources->m_halfResolutionCocTexA , m_renderResources->m_halfResolutionCocTexB };
		m_simpleDofCocBlurComputePass->execute(m_renderResources->m_fullResolutionCocTexture, cocTextures);
		GLuint dofTextures[] = { m_renderResources->m_halfResolutionDofTexA , m_renderResources->m_halfResolutionDofTexB , m_renderResources->m_halfResolutionDofTexC ,m_renderResources->m_halfResolutionDofTexD };
		m_simpleDofBlurComputePass->execute(colorTexture, m_renderResources->m_halfResolutionCocTexB, dofTextures);
		m_simpleDofFillComputePass->execute(dofTextures + 2);
		m_simpleDofCompositeComputePass->execute(m_renderResources->m_fullResolutionHdrTexture);
		break;
	}
	case DepthOfField::SPRITE_BASED:
	{
		m_spriteDofRenderPass->render(colorTexture, m_renderResources->m_gDepthStencilTexture, m_renderResources->m_fullResolutionCocTexture, m_renderResources->m_halfResolutionDofDoubleTex, &previousRenderPass);
		m_spriteDofCompositeComputePass->execute(m_renderResources->m_fullResolutionHdrTexture);
		break;
	}
	case DepthOfField::TILE_BASED_COMBINED:
	case DepthOfField::TILE_BASED_SEPERATE:
	{
		m_cocTileMaxRenderPass->render(m_renderResources->m_fullResolutionCocTexture, m_renderResources->m_cocTexTmp, m_renderResources->m_cocMaxTex, dofTileSize, &previousRenderPass);
		m_cocNeighborTileMaxRenderPass->render(m_renderResources->m_cocMaxTex, m_renderResources->m_cocNeighborMaxTex, &previousRenderPass);

		m_seperateDofDownsampleComputePass->execute(colorTexture, m_renderResources->m_fullResolutionCocTexture, m_renderResources->m_halfResolutionCocTexA, m_renderResources->m_halfResolutionDofTexA, m_renderResources->m_halfResolutionDofTexB);
		//seperateDofTileMaxComputePass->execute(halfResolutionCocTexA);
		GLuint dofTextures[] = { m_renderResources->m_halfResolutionDofTexA , m_renderResources->m_halfResolutionDofTexB , m_renderResources->m_halfResolutionDofTexC , m_renderResources->m_halfResolutionDofTexD };
		m_seperateDofBlurComputePass->execute(dofTextures, m_renderResources->m_halfResolutionCocTexA, m_renderResources->m_cocNeighborMaxTex);
		m_seperateDofFillComputePass->execute(dofTextures);
		m_seperateDofCompositeComputePass->execute(colorTexture, m_renderResources->m_fullResolutionCocTexture, m_renderResources->m_fullResolutionHdrTexture);
		break;
	}
	default:
		break;
	}

	if (true)
	{
		currentLuminanceTexture = !currentLuminanceTexture;

		m_luminanceGenComputePass->execute(effects, colorTexture, m_renderResources->m_luminanceTempTexture);
		m_luminanceAdaptionComputePass->execute(effects, m_renderResources->m_luminanceTempTexture, m_renderResources->m_luminanceTexture, currentLuminanceTexture);
	}
	else
	{
		currentLuminanceTexture = !currentLuminanceTexture;

		// example min/max: -8 .. 4   means a range from 1/256 to 4  pow(2,-8) .. pow(2,4)
		float histogramLogMin = -8;
		float histogramLogMax = 16;
		histogramLogMin = glm::min(histogramLogMin, histogramLogMax - 1);

		float deltaLog = histogramLogMax - histogramLogMin;
		float multiply = 1.0f / deltaLog;
		float add = -histogramLogMin * multiply;
		glm::vec2 params = glm::vec2(multiply, add);

		m_luminanceHistogramComputePass->execute(colorTexture, m_renderResources->m_luminanceHistogramIntermediary, params);
		m_luminanceHistogramReduceComputePass->execute(m_renderResources->m_luminanceHistogram);
		m_luminanceHistogramAdaptionComputePass->execute(m_renderResources->m_luminanceHistogram, m_renderResources->m_luminanceTexture, currentLuminanceTexture, params);
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, effects.depthOfField != DepthOfField::OFF ? m_renderResources->m_fullResolutionHdrTexture : colorTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_renderResources->m_gDepthStencilTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_renderResources->m_halfResolutionHdrTexB);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_renderResources->m_halfResolutionHdrTexA);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, m_renderResources->m_gVelocityTexture);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, m_renderResources->m_velocityNeighborMaxTex);
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, m_renderResources->m_luminanceTexture[currentLuminanceTexture]);
	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, m_renderResources->m_halfResolutionGodRayTexB);
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D, m_renderResources->m_anamorphicPrefilter);

	m_toneMapRenderPass->render(effects, glm::dot(glm::vec3(1.0), renderData.viewDirection), &previousRenderPass);

	finishedTexture = m_renderResources->m_fullResolutionTextureA;

	if (effects.fxaa.enabled)
	{
		m_fxaaRenderPass->render(effects, finishedTexture, (finishedTexture == m_renderResources->m_fullResolutionTextureA) ? GL_COLOR_ATTACHMENT1 : GL_COLOR_ATTACHMENT0, &previousRenderPass);
		finishedTexture = (finishedTexture == m_renderResources->m_fullResolutionTextureA) ? m_renderResources->m_fullResolutionTextureB : m_renderResources->m_fullResolutionTextureA;
	}

	if (effects.chromaticAberration.enabled || effects.vignette.enabled || effects.filmGrain.enabled)
	{
		m_simplePostEffectsRenderPass->render(effects, finishedTexture, (finishedTexture == m_renderResources->m_fullResolutionTextureA) ? GL_COLOR_ATTACHMENT1 : GL_COLOR_ATTACHMENT0, &previousRenderPass);
		finishedTexture = (finishedTexture == m_renderResources->m_fullResolutionTextureA) ? m_renderResources->m_fullResolutionTextureB : m_renderResources->m_fullResolutionTextureA;
	}

	if (debugDraw)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_renderResources->m_debugFbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, finishedTexture, 0);
		m_boundingBoxRenderPass->render(renderData, level, scene, &previousRenderPass);
	}

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

void GLRenderer::resize(unsigned int width, unsigned int height)
{
	m_renderResources->resize(width, height);

	m_gBufferRenderPass->resize(width, height);
	m_gBufferCustomRenderPass->resize(width, height);
	m_ssaoOriginalRenderPass->resize(width, height);
	m_ssaoRenderPass->resize(width, height);
	m_hbaoRenderPass->resize(width, height);
	m_gtaoRenderPass->resize(width, height);
	m_gtaoSpatialDenoiseRenderPass->resize(width, height);
	m_gtaoTemporalDenoiseRenderPass->resize(width, height);
	m_ssaoBlurRenderPass->resize(width, height);
	m_ssaoBilateralBlurRenderPass->resize(width, height);
	m_skyboxRenderPass->resize(width, height);
	m_ambientLightRenderPass->resize(width, height);
	m_directionalLightRenderPass->resize(width, height);
	m_stencilRenderPass->resize(width, height);
	m_deferredEnvironmentProbeRenderPass->resize(width, height);
	m_pointLightRenderPass->resize(width, height);
	m_spotLightRenderPass->resize(width, height);
	m_forwardRenderPass->resize(width, height);
	m_forwardCustomRenderPass->resize(width, height);
	m_outlineRenderPass->resize(width, height);
	m_lightProbeRenderPass->resize(width, height);

	m_anamorphicPrefilterComputePass->resize(width, height);
	m_anamorphicDownsampleComputePass->resize(width, height);
	m_anamorphicUpsampleComputePass->resize(width, height);
	m_fxaaRenderPass->resize(width, height);
	m_smaaEdgeDetectionRenderPass->resize(width, height);
	m_smaaBlendWeightRenderPass->resize(width, height);
	m_smaaBlendRenderPass->resize(width, height);
	m_smaaTemporalResolveRenderPass->resize(width, height);
	m_godRayMaskComputePass->resize(width, height);
	m_godRayGenComputePass->resize(width, height);
	m_luminanceGenComputePass->resize(width, height);
	m_luminanceAdaptionComputePass->resize(width, height);
	m_velocityCorrectionComputePass->resize(width, height);
	m_simpleDofCocBlurComputePass->resize(width, height);
	m_simpleDofBlurComputePass->resize(width, height);
	m_simpleDofFillComputePass->resize(width, height);
	m_simpleDofCompositeComputePass->resize(width, height);
	m_spriteDofRenderPass->resize(width, height / 2);
	m_spriteDofCompositeComputePass->resize(width, height);
	m_seperateDofDownsampleComputePass->resize(width, height);
	m_seperateDofBlurComputePass->resize(width, height);
	m_seperateDofFillComputePass->resize(width, height);
	m_seperateDofCompositeComputePass->resize(width, height);
	m_luminanceHistogramComputePass->resize(width, height);
	m_luminanceHistogramReduceComputePass->resize(width, height);
	m_luminanceHistogramAdaptionComputePass->resize(width, height);
	m_cocComputePass->resize(width, height);
	m_cocTileMaxRenderPass->resize(width, height);
	m_cocNeighborTileMaxRenderPass->resize(width / dofTileSize, height / dofTileSize);
	m_velocityTileMaxRenderPass->resize(width, height);
	m_velocityNeighborTileMaxRenderPass->resize(width / mbTileSize, height / mbTileSize);
	m_lensFlareGenRenderPass->resize(width, height);
	m_lensFlareBlurRenderPass->resize(width, height);
	m_bloomDownsampleComputePass->resize(width, height);
	m_bloomUpsampleComputePass->resize(width, height);
	m_simplePostEffectsRenderPass->resize(width, height);
	m_toneMapRenderPass->resize(width, height);
	m_combinedDofTileMaxComputePass->resize(width, height);
	m_combinedDofNeighborTileMaxComputePass->resize(width, height);
	m_seperateDofTileMaxComputePass->resize(width, height);
	m_antiAliasingTonemapComputePass->resize(width, height);
	m_antiAliasingReverseTonemapComputePass->resize(width, height);

	m_boundingBoxRenderPass->resize(width, height);
}

GLuint GLRenderer::getColorTexture() const
{
	return m_renderResources->m_gLightColorTextures[frame % 2];
}

GLuint GLRenderer::getAlbedoTexture() const
{
	return m_renderResources->m_gAlbedoTexture;
}

GLuint GLRenderer::getNormalTexture() const
{
	return m_renderResources->m_gNormalTexture;
}

GLuint GLRenderer::getMaterialTexture() const
{
	return m_renderResources->m_gMRASTexture;
}

GLuint GLRenderer::getDepthStencilTexture() const
{
	return m_renderResources->m_gDepthStencilTexture;
}

GLuint GLRenderer::getVelocityTexture() const
{
	return m_renderResources->m_gVelocityTexture;
}

GLuint GLRenderer::getAmbientOcclusionTexture() const
{
	return ssaoTexture;
}

GLuint GLRenderer::getBrdfLUT() const
{
	return m_renderResources->m_brdfLUT;
}

GLuint GLRenderer::getFinishedTexture() const
{
	return finishedTexture;
}
