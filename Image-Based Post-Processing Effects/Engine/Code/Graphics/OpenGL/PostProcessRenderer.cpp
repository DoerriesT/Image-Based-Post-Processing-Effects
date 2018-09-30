#include "PostProcessRenderer.h"
#include "GLUtility.h"
#include "Engine.h"
#include <iostream>
#include "ShaderProgram.h"
#include "Graphics\Camera.h"
#include "Window\Window.h"
#include "Graphics\Mesh.h"
#include "Graphics\Effects.h"
#include "Graphics\Texture.h"
#include "Level.h"
#include "RenderData.h"
#include "ComputePass/LensFlares/AnamorphicPrefilterComputePass.h"
#include "ComputePass/LensFlares/AnamorphicDownsampleComputePass.h"
#include "ComputePass/LensFlares/AnamorphicUpsampleComputePass.h"
#include "RenderPass/AntiAliasing/FXAARenderPass.h"
#include "RenderPass/AntiAliasing/SMAAEdgeDetectionRenderPass.h"
#include "RenderPass/AntiAliasing/SMAABlendWeightRenderPass.h"
#include "RenderPass/AntiAliasing/SMAABlendRenderPass.h"
#include "RenderPass/AntiAliasing/SMAATemporalResolveRenderPass.h"
#include "ComputePass/GodRays/GodRayMaskComputePass.h"
#include "ComputePass/GodRays/GodRayGenComputePass.h"
#include "ComputePass/Exposure/LuminanceGenComputePass.h"
#include "ComputePass/Exposure/LuminanceAdaptionComputePass.h"
#include "ComputePass/MotionBlur/VelocityCorrectionComputePass.h"
#include "ComputePass/DepthOfField/SimpleDofCocBlurComputePass.h"
#include "ComputePass/DepthOfField/SimpleDofBlurComputePass.h"
#include "ComputePass/DepthOfField/SimpleDofFillComputePass.h"
#include "ComputePass/DepthOfField/SimpleDofCompositeComputePass.h"
#include "RenderPass/DepthOfField/SpriteDofRenderPass.h"
#include "ComputePass/DepthOfField/SpriteDofCompositeComputePass.h"
#include "ComputePass/DepthOfField/SeperateDofDownsampleComputePass.h"
#include "ComputePass/DepthOfField/SeperateDofBlurComputePass.h"
#include "ComputePass/DepthOfField/SeperateDofFillComputePass.h"
#include "ComputePass/DepthOfField/SeperateDofCompositeComputePass.h"
#include "ComputePass/DepthOfField/CocComputePass.h"
#include "RenderPass/DepthOfField/CocTileMaxRenderPass.h"
#include "RenderPass/DepthOfField/CocNeighborTileMaxRenderPass.h"
#include "ComputePass/Exposure/LuminanceHistogramComputePass.h"
#include "ComputePass/Exposure/LuminanceHistogramReduceComputePass.h"
#include "ComputePass/Exposure/LuminanceHistogramAdaptionComputePass.h"
#include "RenderPass/MotionBlur/VelocityTileMaxRenderPass.h"
#include "RenderPass/MotionBlur/VelocityNeighborTileMaxRenderPass.h"

static const char *BLOOM_ENABLED = "BLOOM_ENABLED";
static const char *FLARES_ENABLED = "FLARES_ENABLED";
static const char *DIRT_ENABLED = "DIRT_ENABLED";
static const char *GOD_RAYS_ENABLED = "GOD_RAYS_ENABLED";
static const char *AUTO_EXPOSURE_ENABLED = "AUTO_EXPOSURE_ENABLED";
static const char *MOTION_BLUR = "MOTION_BLUR";
static const char *ANAMORPHIC_FLARES_ENABLED = "ANAMORPHIC_FLARES_ENABLED";

unsigned int mbTileSize = 40;
unsigned int dofTileSize = 16;
bool godrays = false;

PostProcessRenderer::PostProcessRenderer(std::shared_ptr<Window> _window)
	:window(_window)
{
}

PostProcessRenderer::~PostProcessRenderer()
{
	GLuint textures[] =
	{
		fullResolutionTextureA,
		fullResolutionTextureB,

		halfResolutionHdrTexA,
		halfResolutionHdrTexB,
		halfResolutionHdrTexC,

		resolution4HdrTexA,
		resolution4HdrTexB,

		resolution8HdrTexA,
		resolution8HdrTexB,

		resolution16HdrTexA,
		resolution16HdrTexB,

		resolution32HdrTexA,
		resolution32HdrTexB,

		resolution64HdrTexA,
		resolution64HdrTexB,
	};
	glDeleteTextures(sizeof(textures) / sizeof(GLuint), textures);
	GLuint frameBuffers[] =
	{
		fullResolutionFbo,
		halfResolutionFbo,
		resolution4Fbo,
		resolution8Fbo,
		resolution16Fbo,
		resolution32Fbo,
		resolution64Fbo
	};
	glDeleteFramebuffers(sizeof(frameBuffers) / sizeof(GLuint), frameBuffers);
}

void PostProcessRenderer::init()
{
	// create shaders
	singlePassEffectsShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Misc/singlePassEffects.frag");
	lensFlareGenShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/LensFlares/lensFlareGen.frag");
	lensFlareBlurShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/LensFlares/lensFlareBlur.frag");
	downsampleShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Misc/downsample.frag");
	upsampleShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Misc/upsample.frag");

	hdrShader = ShaderProgram::createShaderProgram(
		{
		{ ShaderProgram::ShaderType::FRAGMENT, BLOOM_ENABLED, 0 },
		{ ShaderProgram::ShaderType::FRAGMENT, FLARES_ENABLED, 0 },
		{ ShaderProgram::ShaderType::FRAGMENT, DIRT_ENABLED, 0 },
		{ ShaderProgram::ShaderType::FRAGMENT, GOD_RAYS_ENABLED, 0 },
		{ ShaderProgram::ShaderType::FRAGMENT, AUTO_EXPOSURE_ENABLED, 0 },
		{ ShaderProgram::ShaderType::FRAGMENT, MOTION_BLUR, 0 },
		},
		"Resources/Shaders/Shared/fullscreenTriangle.vert",
		"Resources/Shaders/Misc/hdr.frag");

	// create uniforms

	// single pass
	uTimeS.create(singlePassEffectsShader);
	uFilmGrainStrengthS.create(singlePassEffectsShader);
	uVignetteS.create(singlePassEffectsShader);
	uFilmGrainS.create(singlePassEffectsShader);
	uChromaticAberrationS.create(singlePassEffectsShader);
	uChromAbOffsetMultiplierS.create(singlePassEffectsShader);

	// hdr
	uStarburstOffsetH.create(hdrShader);
	uBloomStrengthH.create(hdrShader);
	uLensDirtStrengthH.create(hdrShader);
	uExposureH.create(hdrShader);
	uAnamorphicFlareColorH.create(hdrShader);

	// lens flare gen uniforms
	uGhostsLFG.create(lensFlareGenShader);
	uGhostDispersalLFG.create(lensFlareGenShader);
	uHaloRadiusLFG.create(lensFlareGenShader);
	uDistortionLFG.create(lensFlareGenShader);
	uScaleLFG.create(lensFlareGenShader);
	uBiasLFG.create(lensFlareGenShader);

	// lens flare blur shader
	uDirectionLFB.create(lensFlareBlurShader);

	// bloom upscale
	uAddPreviousBU.create(upsampleShader);
	uRadiusBU.create(upsampleShader);

	// create FBO
	glGenFramebuffers(1, &fullResolutionFbo);
	glGenFramebuffers(1, &halfResolutionFbo);
	glGenFramebuffers(1, &resolution4Fbo);
	glGenFramebuffers(1, &resolution8Fbo);
	glGenFramebuffers(1, &resolution16Fbo);
	glGenFramebuffers(1, &resolution32Fbo);
	glGenFramebuffers(1, &resolution64Fbo);
	glGenFramebuffers(1, &velocityFbo);
	glGenFramebuffers(1, &cocFbo);
	glGenFramebuffers(1, &smaaFbo);
	createFboAttachments(std::make_pair(window->getWidth(), window->getHeight()));

	// load textures
	lensColorTexture = Texture::createTexture("Resources/Textures/lenscolor.dds", true);
	lensDirtTexture = Texture::createTexture("Resources/Textures/lensdirt.dds", true);
	lensStarTexture = Texture::createTexture("Resources/Textures/starburst.dds", true);

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);

	anamorphicPrefilterComputePass = new AnamorphicPrefilterComputePass(window->getWidth(), window->getHeight());
	anamorphicDownsampleComputePass = new AnamorphicDownsampleComputePass(window->getWidth(), window->getHeight());
	anamorphicUpsampleComputePass = new AnamorphicUpsampleComputePass(window->getWidth(), window->getHeight());
	fxaaRenderPass = new FXAARenderPass(fullResolutionFbo, window->getWidth(), window->getHeight());
	smaaEdgeDetectionRenderPass = new SMAAEdgeDetectionRenderPass(smaaFbo, window->getWidth(), window->getHeight());
	smaaBlendWeightRenderPass = new SMAABlendWeightRenderPass(smaaFbo, window->getWidth(), window->getHeight());
	smaaBlendRenderPass = new SMAABlendRenderPass(smaaFbo, window->getWidth(), window->getHeight());
	smaaTemporalResolveRenderPass = new SMAATemporalResolveRenderPass(smaaFbo, window->getWidth(), window->getHeight());
	godRayMaskComputePass = new GodRayMaskComputePass(window->getWidth(), window->getHeight());
	godRayGenComputePass = new GodRayGenComputePass(window->getWidth(), window->getHeight());
	luminanceGenComputePass = new LuminanceGenComputePass(window->getWidth(), window->getHeight());
	luminanceAdaptionComputePass = new LuminanceAdaptionComputePass(window->getWidth(), window->getHeight());
	velocityCorrectionComputePass = new VelocityCorrectionComputePass(window->getWidth(), window->getHeight());
	simpleDofCocBlurComputePass = new SimpleDofCocBlurComputePass(window->getWidth(), window->getHeight());
	simpleDofBlurComputePass = new SimpleDofBlurComputePass(window->getWidth(), window->getHeight());
	simpleDofFillComputePass = new SimpleDofFillComputePass(window->getWidth(), window->getHeight());
	simpleDofCompositeComputePass = new SimpleDofCompositeComputePass(window->getWidth(), window->getHeight());
	spriteDofRenderPass = new SpriteDofRenderPass(cocFbo, window->getWidth(), window->getHeight() / 2);
	spriteDofCompositeComputePass = new SpriteDofCompositeComputePass(window->getWidth(), window->getHeight());
	seperateDofDownsampleComputePass = new SeperateDofDownsampleComputePass(window->getWidth(), window->getHeight());
	seperateDofBlurComputePass = new SeperateDofBlurComputePass(window->getWidth(), window->getHeight());
	seperateDofFillComputePass = new SeperateDofFillComputePass(window->getWidth(), window->getHeight());
	seperateDofCompositeComputePass = new SeperateDofCompositeComputePass(window->getWidth(), window->getHeight());
	luminanceHistogramComputePass = new LuminanceHistogramComputePass(window->getWidth(), window->getHeight());
	luminanceHistogramReduceComputePass = new LuminanceHistogramReduceComputePass(window->getWidth(), window->getHeight());
	luminanceHistogramAdaptionComputePass = new LuminanceHistogramAdaptionComputePass(window->getWidth(), window->getHeight());
	cocComputePass = new CocComputePass(window->getWidth(), window->getHeight());
	cocTileMaxRenderPass = new CocTileMaxRenderPass(cocFbo, window->getWidth(), window->getHeight());
	cocNeighborTileMaxRenderPass = new CocNeighborTileMaxRenderPass(cocFbo, window->getWidth() / dofTileSize, window->getHeight() / dofTileSize);
	velocityTileMaxRenderPass = new VelocityTileMaxRenderPass(velocityFbo, window->getWidth(), window->getHeight());
	velocityNeighborTileMaxRenderPass = new VelocityNeighborTileMaxRenderPass(velocityFbo, window->getWidth() / mbTileSize, window->getHeight() / mbTileSize);

}

void PostProcessRenderer::render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const Effects &_effects, GLuint _colorTexture, GLuint _depthTexture, GLuint _velocityTexture, const std::shared_ptr<Camera> &_camera)
{
	if (_effects.smaa.enabled)
	{
		smaa(_effects, _colorTexture, _velocityTexture, _effects.smaa.temporalAntiAliasing);
		if (_effects.smaa.temporalAntiAliasing)
		{
			_colorTexture = fullResolutionSmaaResultTex;
		}
		else
		{
			_colorTexture = fullResolutionSmaaMLResultTex[currentSmaaTexture];
		}
	}

	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	// downsample/blur -> upsample/blur/combine with previous result
	// use end result as bloom and input for lens flares
	if (_effects.bloom.enabled || _effects.lensFlares.enabled)
	{
		// blurred chain in 1/2 res tex A
		downsample(_colorTexture);
		// combine all downsampled/blurred textures in 1/2 res tex B
		upsample();

		// flares in 1/2 A
		if (_effects.lensFlares.enabled)
		{
			// we still have the proper fbo and viewport set from last upsampling step
			generateFlares(_effects);
		}
	}

	if (_effects.anamorphicFlares.enabled)
	{
		anamorphicFlares(_effects, _colorTexture);
	}

	correctVelocities(_renderData, _velocityTexture, _depthTexture);

	if (_effects.motionBlur != MotionBlur::OFF)
	{
		RenderPass *prev = nullptr;
		velocityTileMaxRenderPass->render(_velocityTexture, velocityTexTmp, velocityMaxTex, mbTileSize, &prev);
		velocityNeighborTileMaxRenderPass->render(velocityMaxTex, velocityNeighborMaxTex, &prev);
	}

	if (godrays && !_level->lights.directionalLights.empty())
	{
		glm::vec2 sunpos = glm::vec2(_renderData.viewProjectionMatrix * glm::vec4(_level->lights.directionalLights[0]->getDirection(), 0.0f)) * 0.5f + 0.5f;
		godRays(_effects, sunpos, _colorTexture, _depthTexture);
	}

	switch (_effects.depthOfField)
	{
	case DepthOfField::OFF:
		break;
	case DepthOfField::SIMPLE:
		calculateCoc(_depthTexture);
		simpleDepthOfField(_colorTexture, _depthTexture);
		break;
	case DepthOfField::SPRITE_BASED:
		calculateCoc(_depthTexture);
		spriteBasedDepthOfField(_colorTexture, _depthTexture);
		break;
	case DepthOfField::TILE_BASED_SEPERATE:
		calculateCoc(_depthTexture);
		tileBasedSeperateFieldDepthOfField(_colorTexture);
		break;
	case DepthOfField::TILE_BASED_COMBINED:
		calculateCoc(_depthTexture);
		tileBasedSeperateFieldDepthOfField(_colorTexture);
		break;
	default:
		break;
	}

	calculateLuminance(_effects, _colorTexture);
	//calculateLuminanceHistogram(_colorTexture);

	// combine and tonemap
	glBindFramebuffer(GL_FRAMEBUFFER, fullResolutionFbo);
	glViewport(0, 0, window->getWidth(), window->getHeight());
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _effects.depthOfField != DepthOfField::OFF ? fullResolutionHdrTexture : _colorTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _depthTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, halfResolutionHdrTexB);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, halfResolutionHdrTexA);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, lensDirtTexture->getId());
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, lensStarTexture->getId());
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, _velocityTexture);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, velocityNeighborMaxTex);
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, luminanceTexture[currentLuminanceTexture]);
	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, halfResolutionGodRayTexB);
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D, anamorphicPrefilter);

	// shader permutations
	{
		const auto curDefines = hdrShader->getDefines();

		bool bloomEnabled = false;
		bool flaresEnabled = false;
		bool dirtEnabled = false;
		bool godRaysEnabled = false;
		bool autoExposureEnabled = false;
		int motionBlur = 0;
		bool anamorphicsFlaresEnabled = false;

		for (const auto &define : curDefines)
		{
			if (std::get<0>(define) == ShaderProgram::ShaderType::FRAGMENT)
			{
				if (std::get<1>(define) == BLOOM_ENABLED && std::get<2>(define))
				{
					bloomEnabled = true;
				}
				else if (std::get<1>(define) == FLARES_ENABLED && std::get<2>(define))
				{
					flaresEnabled = true;
				}
				else if (std::get<1>(define) == DIRT_ENABLED && std::get<2>(define))
				{
					dirtEnabled = true;
				}
				else if (std::get<1>(define) == GOD_RAYS_ENABLED && std::get<2>(define))
				{
					godRaysEnabled = true;
				}
				else if (std::get<1>(define) == AUTO_EXPOSURE_ENABLED && std::get<2>(define))
				{
					autoExposureEnabled = true;
				}
				else if (std::get<1>(define) == MOTION_BLUR)
				{
					motionBlur = std::get<2>(define);
				}
				else if (std::get<1>(define) == ANAMORPHIC_FLARES_ENABLED && std::get<2>(define))
				{
					anamorphicsFlaresEnabled = true;
				}
			}
		}

		if (bloomEnabled != _effects.bloom.enabled
			|| flaresEnabled != _effects.lensFlares.enabled
			|| dirtEnabled != _effects.lensDirt.enabled
			|| godRaysEnabled != godrays
			|| autoExposureEnabled != true
			|| anamorphicsFlaresEnabled != _effects.anamorphicFlares.enabled
			|| motionBlur != int(_effects.motionBlur))
		{
			hdrShader->setDefines(
				{
				{ ShaderProgram::ShaderType::FRAGMENT, BLOOM_ENABLED, _effects.bloom.enabled },
				{ ShaderProgram::ShaderType::FRAGMENT, FLARES_ENABLED, _effects.lensFlares.enabled },
				{ ShaderProgram::ShaderType::FRAGMENT, ANAMORPHIC_FLARES_ENABLED, _effects.anamorphicFlares.enabled },
				{ ShaderProgram::ShaderType::FRAGMENT, DIRT_ENABLED, _effects.lensDirt.enabled },
				{ ShaderProgram::ShaderType::FRAGMENT, GOD_RAYS_ENABLED, godrays },
				{ ShaderProgram::ShaderType::FRAGMENT, AUTO_EXPOSURE_ENABLED, 1 },
				{ ShaderProgram::ShaderType::FRAGMENT, MOTION_BLUR, int(_effects.motionBlur) },
				}
			);
			uStarburstOffsetH.create(hdrShader);
			uBloomStrengthH.create(hdrShader);
			uLensDirtStrengthH.create(hdrShader);
			uExposureH.create(hdrShader);
			uAnamorphicFlareColorH.create(hdrShader);
		}
	}

	hdrShader->bind();

	uStarburstOffsetH.set(glm::dot(glm::vec3(1.0), _camera->getForwardDirection()));
	uBloomStrengthH.set(_effects.bloom.strength);
	uExposureH.set(_effects.exposure);
	uLensDirtStrengthH.set(_effects.lensDirt.strength);
	uAnamorphicFlareColorH.set(_effects.anamorphicFlares.color);

	fullscreenTriangle->getSubMesh()->render();

	finishedTexture = fullResolutionTextureA;

	if (_effects.fxaa.enabled)
	{
		fxaa(_effects);
	}

	if (_effects.chromaticAberration.enabled || _effects.vignette.enabled || _effects.filmGrain.enabled)
	{
		singlePassEffects(_effects);
	}
}

void PostProcessRenderer::resize(const std::pair<unsigned int, unsigned int> &_resolution)
{
	GLuint textures[] =
	{
		fullResolutionTextureA,
		fullResolutionTextureB,

		halfResolutionHdrTexA,
		halfResolutionHdrTexB,
		halfResolutionHdrTexC,

		resolution4HdrTexA,
		resolution4HdrTexB,

		resolution8HdrTexA,
		resolution8HdrTexB,

		resolution16HdrTexA,
		resolution16HdrTexB,

		resolution32HdrTexA,
		resolution32HdrTexB,

		resolution64HdrTexA,
		resolution64HdrTexB,
	};
	glDeleteTextures(sizeof(textures) / sizeof(GLuint), textures);
	createFboAttachments(_resolution);
}

GLuint PostProcessRenderer::getFinishedTexture() const
{
	return finishedTexture;
}

void PostProcessRenderer::fxaa(const Effects &_effects)
{
	GLenum drawBuffer = (finishedTexture == fullResolutionTextureA) ? GL_COLOR_ATTACHMENT1 : GL_COLOR_ATTACHMENT0;

	RenderPass *prev = nullptr;
	fxaaRenderPass->render(_effects, finishedTexture, drawBuffer, &prev);

	finishedTexture = (finishedTexture == fullResolutionTextureA) ? fullResolutionTextureB : fullResolutionTextureA;
}

void PostProcessRenderer::smaa(const Effects &_effects, GLuint _colorTexture, GLuint _velocityTexture, bool _temporalAA)
{
	currentSmaaTexture = !currentSmaaTexture;

	RenderPass *previousRenderPass = nullptr;

	smaaEdgeDetectionRenderPass->render(_effects, _colorTexture, &previousRenderPass);
	smaaBlendWeightRenderPass->render(_effects, fullResolutionSmaaEdgesTex, _temporalAA, currentSmaaTexture, &previousRenderPass);
	smaaBlendRenderPass->render(_effects, _colorTexture, _velocityTexture, fullResolutionSmaaBlendTex, currentSmaaTexture, &previousRenderPass);

	if (!_temporalAA)
	{
		return;
	}

	smaaTemporalResolveRenderPass->render(_effects, fullResolutionSmaaMLResultTex, _velocityTexture, currentSmaaTexture, &previousRenderPass);
}

void PostProcessRenderer::singlePassEffects(const Effects &_effects)
{
	glDrawBuffer((finishedTexture == fullResolutionTextureA) ? GL_COLOR_ATTACHMENT1 : GL_COLOR_ATTACHMENT0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, finishedTexture);

	singlePassEffectsShader->bind();
	uTimeS.set((float)Engine::getTime());
	uFilmGrainStrengthS.set(_effects.filmGrain.strength);
	uVignetteS.set(_effects.vignette.enabled);
	uFilmGrainS.set(_effects.filmGrain.enabled);
	uChromaticAberrationS.set(_effects.chromaticAberration.enabled);
	uChromAbOffsetMultiplierS.set(_effects.chromaticAberration.offsetMultiplier);

	fullscreenTriangle->getSubMesh()->render();
	finishedTexture = (finishedTexture == fullResolutionTextureA) ? fullResolutionTextureB : fullResolutionTextureA;
}

void PostProcessRenderer::downsample(GLuint _colorTexture)
{
	unsigned int windowWidth = window->getWidth();
	unsigned int windowHeight = window->getHeight();

	glm::vec2 viewports[] =
	{
		glm::vec2(windowWidth / 2.0f, windowHeight / 2.0f),
		glm::vec2(windowWidth / 4.0f, windowHeight / 4.0f),
		glm::vec2(windowWidth / 8.0f, windowHeight / 8.0f),
		glm::vec2(windowWidth / 16.0f, windowHeight / 16.0f),
		glm::vec2(windowWidth / 32.0f, windowHeight / 32.0f),
		glm::vec2(windowWidth / 64.0f, windowHeight / 64.0f)
	};

	GLuint colorTextures[] =
	{
		_colorTexture,
		halfResolutionHdrTexA,
		resolution4HdrTexA,
		resolution8HdrTexA,
		resolution16HdrTexA,
		resolution32HdrTexA,
		resolution64HdrTexA,
	};

	GLuint fbos[] =
	{
		halfResolutionFbo,
		resolution4Fbo,
		resolution8Fbo,
		resolution16Fbo,
		resolution32Fbo,
		resolution64Fbo,
	};

	downsampleShader->bind();

	for (size_t i = 0; i < 6; ++i)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbos[i]);
		glViewport(0, 0, (GLsizei)viewports[i].x, (GLsizei)viewports[i].y);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorTextures[i]);

		fullscreenTriangle->getSubMesh()->render();
		//glDrawArrays(GL_TRIANGLES, 0, 3);
	}
}

void PostProcessRenderer::upsample()
{
	unsigned int windowWidth = window->getWidth();
	unsigned int windowHeight = window->getHeight();

	glm::vec2 viewports[] =
	{
		glm::vec2(windowWidth / 64.0f, windowHeight / 64.0f),
		glm::vec2(windowWidth / 32.0f, windowHeight / 32.0f),
		glm::vec2(windowWidth / 16.0f, windowHeight / 16.0f),
		glm::vec2(windowWidth / 8.0f, windowHeight / 8.0f),
		glm::vec2(windowWidth / 4.0f, windowHeight / 4.0f),
		glm::vec2(windowWidth / 2.0f, windowHeight / 2.0f)
	};

	GLuint fbos[] =
	{
		resolution64Fbo,
		resolution32Fbo,
		resolution16Fbo,
		resolution8Fbo,
		resolution4Fbo,
		halfResolutionFbo
	};

	GLuint sourceTextures[] =
	{
		resolution64HdrTexA,
		resolution32HdrTexA,
		resolution16HdrTexA,
		resolution8HdrTexA,
		resolution4HdrTexA,
		halfResolutionHdrTexA
	};

	GLuint targetTextures[] =
	{
		resolution64HdrTexB,
		resolution32HdrTexB,
		resolution16HdrTexB,
		resolution8HdrTexB,
		resolution4HdrTexB,
	};

	float radiusMult[] =
	{
		1.3f, 1.25f, 1.2f, 1.15f, 1.1f, 1.05f
	};

	upsampleShader->bind();
	uAddPreviousBU.set(false);

	for (size_t i = 0; i < 6; ++i)
	{
		// first iteration without previous blur, all other combine with previous
		if (i == 1)
		{
			uAddPreviousBU.set(true);
		}

		if (i > 0)
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, targetTextures[i - 1]);
		}

		uRadiusBU.set((1.0f / viewports[i]) * radiusMult[i]);

		glBindFramebuffer(GL_FRAMEBUFFER, fbos[i]);
		glViewport(0, 0, (GLsizei)viewports[i].x, (GLsizei)viewports[i].y);
		glDrawBuffer(GL_COLOR_ATTACHMENT1);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sourceTextures[i]);

		fullscreenTriangle->getSubMesh()->render();
	}
}

void PostProcessRenderer::generateFlares(const Effects &_effects)
{
	// texture B contains combined blurred mipmap chain
	// generate ghosts by sampling blurred texture with threshold
	glDrawBuffer(GL_COLOR_ATTACHMENT0); // write to A
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, halfResolutionHdrTexB); // read from B
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(lensColorTexture->getTarget(), lensColorTexture->getId());

	lensFlareGenShader->bind();
	uGhostsLFG.set(_effects.lensFlares.flareCount);
	uGhostDispersalLFG.set(_effects.lensFlares.flareSpacing);
	uHaloRadiusLFG.set(_effects.lensFlares.haloWidth);
	uDistortionLFG.set(_effects.lensFlares.chromaticDistortion);

	fullscreenTriangle->getSubMesh()->render();
	//glDrawArrays(GL_TRIANGLES, 0, 3);

	// blur result (maybe skip this step, since source should already be pretty blurry)

	glDrawBuffer(GL_COLOR_ATTACHMENT2); // write to C
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, halfResolutionHdrTexA); // read from A

	lensFlareBlurShader->bind();
	uDirectionLFB.set(true);

	fullscreenTriangle->getSubMesh()->render();
	//glDrawArrays(GL_TRIANGLES, 0, 3);

	glDrawBuffer(GL_COLOR_ATTACHMENT0); // write to A
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, halfResolutionHdrTexC); // read from C

	uDirectionLFB.set(false);

	fullscreenTriangle->getSubMesh()->render();
	//glDrawArrays(GL_TRIANGLES, 0, 3);
}

void PostProcessRenderer::anamorphicFlares(const Effects &_effects, GLuint _colorTexture)
{
	anamorphicPrefilterComputePass->execute(_effects, _colorTexture, anamorphicPrefilter);
	int lastUsedTexture;
	unsigned int lastWidth;
	anamorphicDownsampleComputePass->execute(_effects, anamorphicPrefilter, anamorphicChain, 6, lastUsedTexture, lastWidth);
	anamorphicUpsampleComputePass->execute(_effects, anamorphicPrefilter, anamorphicChain, 6, lastUsedTexture, lastWidth);
}

void PostProcessRenderer::calculateCoc(GLuint _depthTexture)
{
	cocComputePass->execute(_depthTexture, fullResolutionCocTexture, window->getFieldOfView(), Window::NEAR_PLANE, Window::FAR_PLANE);
}

void PostProcessRenderer::calculateCocTileTexture()
{
	RenderPass *prev = nullptr;
	cocTileMaxRenderPass->render(fullResolutionCocTexture, cocTexTmp, cocMaxTex, dofTileSize, &prev);
	cocNeighborTileMaxRenderPass->render(cocMaxTex, cocNeighborMaxTex, &prev);
}

void PostProcessRenderer::simpleDepthOfField(GLuint _colorTexture, GLuint _depthTexture)
{
	GLuint cocTextures[] = { halfResolutionCocTexA , halfResolutionCocTexB };
	simpleDofCocBlurComputePass->execute(fullResolutionCocTexture, cocTextures);
	GLuint dofTextures[] = { halfResolutionDofTexA , halfResolutionDofTexB , halfResolutionDofTexC , halfResolutionDofTexD };
	simpleDofBlurComputePass->execute(_colorTexture, halfResolutionCocTexB, dofTextures);
	simpleDofFillComputePass->execute(dofTextures + 2);
	simpleDofCompositeComputePass->execute(fullResolutionHdrTexture);
}

void PostProcessRenderer::tileBasedSeperateFieldDepthOfField(GLuint _colorTexture)
{
	calculateCocTileTexture();

	seperateDofDownsampleComputePass->execute(_colorTexture, fullResolutionCocTexture, halfResolutionCocTexA, halfResolutionDofTexA, halfResolutionDofTexB);
	GLuint dofTextures[] = { halfResolutionDofTexA , halfResolutionDofTexB , halfResolutionDofTexC , halfResolutionDofTexD };
	seperateDofBlurComputePass->execute(dofTextures, halfResolutionCocTexA, cocNeighborMaxTex);
	seperateDofFillComputePass->execute(dofTextures);
	seperateDofCompositeComputePass->execute(_colorTexture, fullResolutionCocTexture, fullResolutionHdrTexture);
}

void PostProcessRenderer::spriteBasedDepthOfField(GLuint _colorTexture, GLuint _depthTexture)
{
	RenderPass *prev = nullptr;
	spriteDofRenderPass->render(_colorTexture, _depthTexture, fullResolutionCocTexture, halfResolutionDofDoubleTex, &prev);
	spriteDofCompositeComputePass->execute(fullResolutionHdrTexture);

	glDisable(GL_BLEND);
}

void PostProcessRenderer::godRays(const Effects &_effects, const glm::vec2 &_sunpos, GLuint _colorTexture, GLuint _depthTexture)
{
	GLuint godRayTextures[] = { halfResolutionGodRayTexA, halfResolutionGodRayTexB };
	godRayMaskComputePass->execute(_effects, _colorTexture, _depthTexture, godRayTextures[0]);
	godRayGenComputePass->execute(_effects, godRayTextures, _sunpos);
}

void PostProcessRenderer::calculateLuminance(const Effects &_effects, GLuint _colorTexture)
{
	currentLuminanceTexture = !currentLuminanceTexture;

	luminanceGenComputePass->execute(_effects, _colorTexture, luminanceTempTexture);
	luminanceAdaptionComputePass->execute(_effects, luminanceTempTexture, luminanceTexture, currentLuminanceTexture);
}

void PostProcessRenderer::calculateLuminanceHistogram(GLuint _colorTexture)
{
	currentLuminanceTexture = !currentLuminanceTexture;

	unsigned int width = window->getWidth();
	unsigned int height = window->getHeight();

	// example min/max: -8 .. 4   means a range from 1/256 to 4  pow(2,-8) .. pow(2,4)
	float histogramLogMin = -8;
	float histogramLogMax = 16;
	histogramLogMin = glm::min(histogramLogMin, histogramLogMax - 1);

	float deltaLog = histogramLogMax - histogramLogMin;
	float multiply = 1.0f / deltaLog;
	float add = -histogramLogMin * multiply;
	glm::vec2 params = glm::vec2(multiply, add);

	luminanceHistogramComputePass->execute(_colorTexture, luminanceHistogramIntermediary, params);
	luminanceHistogramReduceComputePass->execute(luminanceHistogram);
	luminanceHistogramAdaptionComputePass->execute(luminanceHistogram, luminanceTexture, currentLuminanceTexture, params);
}

void PostProcessRenderer::correctVelocities(const RenderData & _renderData, GLuint _velocityTexture, GLuint _depthTexture)
{
	velocityCorrectionComputePass->execute(_renderData, _velocityTexture, _depthTexture);
}

void PostProcessRenderer::createFboAttachments(const std::pair<unsigned int, unsigned int> &_resolution)
{
	// full res
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fullResolutionFbo);

		glGenTextures(1, &fullResolutionTextureA);
		glBindTexture(GL_TEXTURE_2D, fullResolutionTextureA);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _resolution.first, _resolution.second, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fullResolutionTextureA, 0);

		glGenTextures(1, &fullResolutionTextureB);
		glBindTexture(GL_TEXTURE_2D, fullResolutionTextureB);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _resolution.first, _resolution.second, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, fullResolutionTextureB, 0);

		glGenTextures(1, &fullResolutionHdrTexture);
		glBindTexture(GL_TEXTURE_2D, fullResolutionHdrTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _resolution.first, _resolution.second, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, fullResolutionHdrTexture, 0);

		glGenTextures(1, &fullResolutionCocTexture);
		glBindTexture(GL_TEXTURE_2D, fullResolutionCocTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _resolution.first, _resolution.second, 0, GL_RG, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenTextures(1, &fullResolutionDofTexA);
		glBindTexture(GL_TEXTURE_2D, fullResolutionDofTexA);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _resolution.first, _resolution.second, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenTextures(1, &fullResolutionDofTexB);
		glBindTexture(GL_TEXTURE_2D, fullResolutionDofTexB);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _resolution.first, _resolution.second, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenTextures(1, &fullResolutionDofTexC);
		glBindTexture(GL_TEXTURE_2D, fullResolutionDofTexC);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _resolution.first, _resolution.second, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenTextures(1, &fullResolutionDofTexD);
		glBindTexture(GL_TEXTURE_2D, fullResolutionDofTexD);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _resolution.first, _resolution.second, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		glBindFramebuffer(GL_FRAMEBUFFER, smaaFbo);

		glGenTextures(1, &fullResolutionSmaaEdgesTex);
		glBindTexture(GL_TEXTURE_2D, fullResolutionSmaaEdgesTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, _resolution.first, _resolution.second, 0, GL_RG, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fullResolutionSmaaEdgesTex, 0);

		glGenTextures(1, &fullResolutionSmaaBlendTex);
		glBindTexture(GL_TEXTURE_2D, fullResolutionSmaaBlendTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _resolution.first, _resolution.second, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, fullResolutionSmaaBlendTex, 0);

		glGenTextures(1, &fullResolutionSmaaMLResultTex[0]);
		glBindTexture(GL_TEXTURE_2D, fullResolutionSmaaMLResultTex[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _resolution.first, _resolution.second, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, fullResolutionSmaaMLResultTex[0], 0);

		glGenTextures(1, &fullResolutionSmaaMLResultTex[1]);
		glBindTexture(GL_TEXTURE_2D, fullResolutionSmaaMLResultTex[1]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _resolution.first, _resolution.second, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, fullResolutionSmaaMLResultTex[1], 0);

		glGenTextures(1, &fullResolutionSmaaResultTex);
		glBindTexture(GL_TEXTURE_2D, fullResolutionSmaaResultTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _resolution.first, _resolution.second, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, fullResolutionSmaaResultTex, 0);

		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "Full Res fbo not complete!" << std::endl;
		}
	}

	// half res
	{
		glBindFramebuffer(GL_FRAMEBUFFER, halfResolutionFbo);

		glGenTextures(1, &halfResolutionHdrTexA);
		glBindTexture(GL_TEXTURE_2D, halfResolutionHdrTexA);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, _resolution.first / 2, _resolution.second / 2, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, halfResolutionHdrTexA, 0);

		glGenTextures(1, &halfResolutionHdrTexB);
		glBindTexture(GL_TEXTURE_2D, halfResolutionHdrTexB);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, _resolution.first / 2, _resolution.second / 2, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, halfResolutionHdrTexB, 0);

		glGenTextures(1, &halfResolutionHdrTexC);
		glBindTexture(GL_TEXTURE_2D, halfResolutionHdrTexC);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, _resolution.first / 2, _resolution.second / 2, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, halfResolutionHdrTexC, 0);

		glGenTextures(1, &halfResolutionDofTexA);
		glBindTexture(GL_TEXTURE_2D, halfResolutionDofTexA);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _resolution.first / 2, _resolution.second / 2, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenTextures(1, &halfResolutionDofTexB);
		glBindTexture(GL_TEXTURE_2D, halfResolutionDofTexB);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _resolution.first / 2, _resolution.second / 2, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenTextures(1, &halfResolutionDofTexC);
		glBindTexture(GL_TEXTURE_2D, halfResolutionDofTexC);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _resolution.first / 2, _resolution.second / 2, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenTextures(1, &halfResolutionDofTexD);
		glBindTexture(GL_TEXTURE_2D, halfResolutionDofTexD);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _resolution.first / 2, _resolution.second / 2, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenTextures(1, &halfResolutionDofDoubleTex);
		glBindTexture(GL_TEXTURE_2D, halfResolutionDofDoubleTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _resolution.first, _resolution.second / 2, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenTextures(1, &halfResolutionCocTexA);
		glBindTexture(GL_TEXTURE_2D, halfResolutionCocTexA);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _resolution.first / 2, _resolution.second / 2, 0, GL_RG, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenTextures(1, &halfResolutionCocTexB);
		glBindTexture(GL_TEXTURE_2D, halfResolutionCocTexB);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _resolution.first / 2, _resolution.second / 2, 0, GL_RG, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenTextures(1, &halfResolutionGodRayTexA);
		glBindTexture(GL_TEXTURE_2D, halfResolutionGodRayTexA);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _resolution.first / 2, _resolution.second / 2, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenTextures(1, &halfResolutionGodRayTexB);
		glBindTexture(GL_TEXTURE_2D, halfResolutionGodRayTexB);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _resolution.first / 2, _resolution.second / 2, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		GLUtility::glErrorCheck("");

		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "1/2 Res Framebuffer not complete!" << std::endl;
		}
	}

	// 1/4 res
	{
		glBindFramebuffer(GL_FRAMEBUFFER, resolution4Fbo);

		glGenTextures(1, &resolution4HdrTexA);
		glBindTexture(GL_TEXTURE_2D, resolution4HdrTexA);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, _resolution.first / 4, _resolution.second / 4, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolution4HdrTexA, 0);

		glGenTextures(1, &resolution4HdrTexB);
		glBindTexture(GL_TEXTURE_2D, resolution4HdrTexB);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, _resolution.first / 4, _resolution.second / 4, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, resolution4HdrTexB, 0);

		GLUtility::glErrorCheck("");
	}

	// 1/8 res
	{
		glBindFramebuffer(GL_FRAMEBUFFER, resolution8Fbo);


		glGenTextures(1, &resolution8HdrTexA);
		glBindTexture(GL_TEXTURE_2D, resolution8HdrTexA);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, _resolution.first / 8, _resolution.second / 8, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolution8HdrTexA, 0);

		glGenTextures(1, &resolution8HdrTexB);
		glBindTexture(GL_TEXTURE_2D, resolution8HdrTexB);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, _resolution.first / 8, _resolution.second / 8, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, resolution8HdrTexB, 0);


		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "1/8 Res Framebuffer not complete!" << std::endl;
		}
	}


	// 1/16 res
	{
		glBindFramebuffer(GL_FRAMEBUFFER, resolution16Fbo);


		glGenTextures(1, &resolution16HdrTexA);
		glBindTexture(GL_TEXTURE_2D, resolution16HdrTexA);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, _resolution.first / 16, _resolution.second / 16, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolution16HdrTexA, 0);

		glGenTextures(1, &resolution16HdrTexB);
		glBindTexture(GL_TEXTURE_2D, resolution16HdrTexB);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, _resolution.first / 16, _resolution.second / 16, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, resolution16HdrTexB, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "1/16 Res Framebuffer not complete!" << std::endl;
		}
	}

	// 1/32 res
	{
		glBindFramebuffer(GL_FRAMEBUFFER, resolution32Fbo);

		glGenTextures(1, &resolution32HdrTexA);
		glBindTexture(GL_TEXTURE_2D, resolution32HdrTexA);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, _resolution.first / 32, _resolution.second / 32, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolution32HdrTexA, 0);

		glGenTextures(1, &resolution32HdrTexB);
		glBindTexture(GL_TEXTURE_2D, resolution32HdrTexB);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, _resolution.first / 32, _resolution.second / 32, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, resolution32HdrTexB, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "1/32 Res Framebuffer not complete!" << std::endl;
		}
	}

	// 1/64 res
	{
		glBindFramebuffer(GL_FRAMEBUFFER, resolution64Fbo);

		glGenTextures(1, &resolution64HdrTexA);
		glBindTexture(GL_TEXTURE_2D, resolution64HdrTexA);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, _resolution.first / 64, _resolution.second / 64, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolution64HdrTexA, 0);

		glGenTextures(1, &resolution64HdrTexB);
		glBindTexture(GL_TEXTURE_2D, resolution64HdrTexB);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, _resolution.first / 64, _resolution.second / 64, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, resolution64HdrTexB, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "1/64 Res Framebuffer not complete!" << std::endl;
		}
	}

	// velocity
	{
		glBindFramebuffer(GL_FRAMEBUFFER, velocityFbo);

		glGenTextures(1, &velocityTexTmp);
		glBindTexture(GL_TEXTURE_2D, velocityTexTmp);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _resolution.first / mbTileSize, _resolution.second, 0, GL_RG, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, velocityTexTmp, 0);

		glGenTextures(1, &velocityMaxTex);
		glBindTexture(GL_TEXTURE_2D, velocityMaxTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _resolution.first / mbTileSize, _resolution.second / mbTileSize, 0, GL_RG, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenTextures(1, &velocityNeighborMaxTex);
		glBindTexture(GL_TEXTURE_2D, velocityNeighborMaxTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _resolution.first / mbTileSize, _resolution.second / mbTileSize, 0, GL_RG, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "velocity Framebuffer not complete!" << std::endl;
		}
	}

	// coc
	{
		glBindFramebuffer(GL_FRAMEBUFFER, cocFbo);

		glGenTextures(1, &cocTexTmp);
		glBindTexture(GL_TEXTURE_2D, cocTexTmp);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _resolution.first / dofTileSize, _resolution.second, 0, GL_RG, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cocTexTmp, 0);

		glGenTextures(1, &cocMaxTex);
		glBindTexture(GL_TEXTURE_2D, cocMaxTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _resolution.first / dofTileSize, _resolution.second / dofTileSize, 0, GL_RG, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenTextures(1, &cocNeighborMaxTex);
		glBindTexture(GL_TEXTURE_2D, cocNeighborMaxTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _resolution.first / dofTileSize, _resolution.second / dofTileSize, 0, GL_RG, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "velocity Framebuffer not complete!" << std::endl;
		}
	}

	// luminance
	{
		glGenTextures(1, &luminanceTempTexture);
		glBindTexture(GL_TEXTURE_2D, luminanceTempTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, _resolution.first, _resolution.second, 0, GL_RED, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenTextures(2, luminanceTexture);
		glBindTexture(GL_TEXTURE_2D, luminanceTexture[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, 1, 1, 0, GL_RED, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_2D, luminanceTexture[1]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, 1, 1, 0, GL_RED, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	// luminance histogram
	{
		unsigned int numGroupsX = _resolution.first / 8 + ((_resolution.first % 8 == 0) ? 0 : 1);
		unsigned int numGroupsY = _resolution.second / 8 + ((_resolution.second % 8 == 0) ? 0 : 1);
		numGroupsX = numGroupsX / 8 + ((numGroupsX % 8 == 0) ? 0 : 1);
		numGroupsY = numGroupsY / 8 + ((numGroupsY % 8 == 0) ? 0 : 1);

		glGenTextures(1, &luminanceHistogramIntermediary);
		glBindTexture(GL_TEXTURE_2D, luminanceHistogramIntermediary);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 64 / 4, numGroupsX * numGroupsY, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenTextures(1, &luminanceHistogram);
		glBindTexture(GL_TEXTURE_2D, luminanceHistogram);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 64 / 4, 1, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	// anamorphic
	{
		glGenTextures(1, &anamorphicPrefilter);
		glBindTexture(GL_TEXTURE_2D, anamorphicPrefilter);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, _resolution.first, _resolution.second / 2, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		unsigned int width = _resolution.first;
		for (unsigned int i = 0; i < 6; ++i)
		{
			width /= 2;

			glGenTextures(1, &anamorphicChain[i]);
			glBindTexture(GL_TEXTURE_2D, anamorphicChain[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, width, _resolution.second / 2, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
