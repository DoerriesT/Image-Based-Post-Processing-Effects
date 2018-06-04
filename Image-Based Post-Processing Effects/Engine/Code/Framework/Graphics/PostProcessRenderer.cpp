#include "PostProcessRenderer.h"
#include "Utilities\Utility.h"
#include ".\..\..\Engine.h"
#include <iostream>
#include "ShaderProgram.h"
#include ".\..\..\Graphics\Camera.h"
#include ".\..\..\Window.h"
#include ".\..\..\Graphics\Mesh.h"
#include ".\..\..\Graphics\Effects.h"
#include ".\..\..\Graphics\Texture.h"

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
	singlePassEffectsShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/PostProcess/singlePassEffects.frag");
	hdrShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/PostProcess/hdr.frag");
	fxaaShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/PostProcess/fxaa.frag");
	lensFlareGenShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/PostProcess/lensFlareGen.frag");
	lensFlareBlurShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/PostProcess/lensFlareBlur.frag");
	downsampleShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/PostProcess/downsample.frag");
	upsampleShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/PostProcess/upsample.frag");
	velocityTileMaxShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/PostProcess/velocityTileMax.frag");
	velocityNeighborTileMaxShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/PostProcess/velocityNeighborTileMax.frag");
	cocShader = ShaderProgram::createShaderProgram("Resources/Shaders/PostProcess/coc.comp");
	cocBlurShader = ShaderProgram::createShaderProgram("Resources/Shaders/PostProcess/cocBlur.comp");
	dofBlurShader = ShaderProgram::createShaderProgram("Resources/Shaders/PostProcess/dofBlur.comp");
	dofFillShader = ShaderProgram::createShaderProgram("Resources/Shaders/PostProcess/dofFill.comp");

	// create uniforms

	// single pass
	uScreenTextureS.create(singlePassEffectsShader);
	uTimeS.create(singlePassEffectsShader);
	uFilmGrainStrengthS.create(singlePassEffectsShader);
	uVignetteS.create(singlePassEffectsShader);
	uFilmGrainS.create(singlePassEffectsShader);
	uChromaticAberrationS.create(singlePassEffectsShader);
	uChromAbOffsetMultiplierS.create(singlePassEffectsShader);

	// hdr
	uScreenTextureH.create(hdrShader);
	uBloomTextureH.create(hdrShader);
	uLensFlareTexH.create(hdrShader);
	uLensDirtTexH.create(hdrShader);
	uLensStarTexH.create(hdrShader);
	uStarburstOffsetH.create(hdrShader);
	uLensFlaresH.create(hdrShader);
	uBloomH.create(hdrShader);
	uBloomStrengthH.create(hdrShader);
	uBloomDirtStrengthH.create(hdrShader);
	uExposureH.create(hdrShader);
	uVelocityTextureH.create(hdrShader);
	uVelocityScaleH.create(hdrShader);
	uMotionBlurH.create(hdrShader);
	uVelocityNeighborMaxTextureH.create(hdrShader);
	uDepthTextureH.create(hdrShader);
	uDofNearTextureH.create(hdrShader);
	uDofFarTextureH.create(hdrShader);

	// fxaa
	uScreenTextureF.create(fxaaShader);
	uInverseResolutionF.create(fxaaShader);
	uSubPixelAAF.create(fxaaShader);
	uEdgeThresholdF.create(fxaaShader);
	uEdgeThresholdMinF.create(fxaaShader);

	// lens flare gen uniforms
	uInputTexLFG.create(lensFlareGenShader);
	uLensColorLFG.create(lensFlareGenShader);
	uGhostsLFG.create(lensFlareGenShader);
	uGhostDispersalLFG.create(lensFlareGenShader);
	uHaloRadiusLFG.create(lensFlareGenShader);
	uDistortionLFG.create(lensFlareGenShader);
	uScaleLFG.create(lensFlareGenShader);
	uBiasLFG.create(lensFlareGenShader);

	// lens flare blur shader
	uInputTexLFB.create(lensFlareBlurShader);
	uDirectionLFB.create(lensFlareBlurShader);

	// downsample
	uColorTextureDS2.create(downsampleShader);

	// bloom upscale
	uUpscaleTextureBU.create(upsampleShader);
	uPreviousBlurredTextureBU.create(upsampleShader);
	uAddPreviousBU.create(upsampleShader);
	uRadiusBU.create(upsampleShader);

	// velocity tile max
	uVelocityTextureVTM.create(velocityTileMaxShader);
	uDirectionVTM.create(velocityTileMaxShader);
	uTileSizeVTM.create(velocityTileMaxShader);

	// velocity neighbor tile max
	uVelocityTextureVNTM.create(velocityNeighborTileMaxShader);

	// coc
	uDepthTextureCOC.create(cocShader);
	uFocusDistanceCOC.create(cocShader);
	uFocalLengthCOC.create(cocShader);

	// coc blur
	uCocTextureCOCB.create(cocBlurShader);
	uDirectionCOCB.create(cocBlurShader);

	// dof blur
	uColorTextureDOFB.create(dofBlurShader);
	uCocTextureDOFB.create(dofBlurShader);
	for (int i = 0; i < 64; ++i)
	{
		uSampleCoordsDOFB.push_back(dofBlurShader->createUniform(std::string("uSampleCoords") + "[" + std::to_string(i) + "]"));
	}
	uBokehScaleDOFB.create(dofBlurShader);

	// dof fill
	uColorNearTextureDOFF.create(dofFillShader);
	uColorFarTextureDOFF.create(dofFillShader);
	for (int i = 0; i < 16; ++i)
	{
		uSampleCoordsDOFF.push_back(dofFillShader->createUniform(std::string("uSampleCoords") + "[" + std::to_string(i) + "]"));
	}
	uBokehScaleDOFF.create(dofFillShader);

	// create FBO
	glGenFramebuffers(1, &fullResolutionFbo);
	glGenFramebuffers(1, &halfResolutionFbo);
	glGenFramebuffers(1, &resolution4Fbo);
	glGenFramebuffers(1, &resolution8Fbo);
	glGenFramebuffers(1, &resolution16Fbo);
	glGenFramebuffers(1, &resolution32Fbo);
	glGenFramebuffers(1, &resolution64Fbo);
	glGenFramebuffers(1, &velocityFbo);
	createFboAttachments(std::make_pair(window->getWidth(), window->getHeight()));

	// load textures
	lensColorTexture = Texture::createTexture("Resources/Textures/lenscolor.dds", true);
	lensDirtTexture = Texture::createTexture("Resources/Textures/lensdirt.dds", true);
	lensStarTexture = Texture::createTexture("Resources/Textures/starburst.dds", true);

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

unsigned int tileSize = 40;

void PostProcessRenderer::render(const Effects &_effects, GLuint _colorTexture, GLuint _depthTexture, GLuint _velocityTexture, const std::shared_ptr<Camera> &_camera)
{
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

	if (_effects.motionBlur != MotionBlur::OFF)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, velocityFbo);
		glActiveTexture(GL_TEXTURE0);

		// tile max
		{
			velocityTileMaxShader->bind();

			uVelocityTextureVTM.set(0);
			uTileSizeVTM.set(tileSize);

			// fullscreen to first step
			{
				glViewport(0, 0, window->getWidth() / tileSize, window->getHeight());
				glBindTexture(GL_TEXTURE_2D, _velocityTexture);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, velocityTexTmp, 0);

				uDirectionVTM.set(false);

				fullscreenTriangle->getSubMesh()->render();
			}

			// first to second step
			{
				glViewport(0, 0, window->getWidth() / tileSize, window->getHeight() / tileSize);
				glBindTexture(GL_TEXTURE_2D, velocityTexTmp);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, velocityMaxTex, 0);

				uDirectionVTM.set(true);

				fullscreenTriangle->getSubMesh()->render();
			}
		}

		// tile neighbor max
		{
			velocityNeighborTileMaxShader->bind();

			uVelocityTextureVNTM.set(0);

			glBindTexture(GL_TEXTURE_2D, velocityMaxTex);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, velocityNeighborMaxTex, 0);

			fullscreenTriangle->getSubMesh()->render();
		}

	}

	simpleDepthOfField(_colorTexture, _depthTexture);

	// combine and tonemap
	glBindFramebuffer(GL_FRAMEBUFFER, fullResolutionFbo);
	glViewport(0, 0, window->getWidth(), window->getHeight());
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _colorTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, halfResolutionHdrTexB);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, halfResolutionHdrTexA);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, lensDirtTexture->getId());
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, lensStarTexture->getId());
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, _velocityTexture);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, velocityNeighborMaxTex);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, _depthTexture);
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, halfResolutionDofTexC);
	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, halfResolutionDofTexD);

	hdrShader->bind();
	uScreenTextureH.set(0);
	uBloomTextureH.set(1);
	uLensFlareTexH.set(2);
	uLensDirtTexH.set(3);
	uLensStarTexH.set(4);
	uVelocityTextureH.set(5);
	uVelocityNeighborMaxTextureH.set(6);
	uDepthTextureH.set(7);
	uDofNearTextureH.set(8);
	uDofFarTextureH.set(9);

	uStarburstOffsetH.set(glm::dot(glm::vec3(1.0), _camera->getForwardDirection()));
	uLensFlaresH.set(_effects.lensFlares.enabled);
	uBloomH.set(_effects.bloom.enabled);
	uBloomStrengthH.set(_effects.bloom.strength);
	uBloomDirtStrengthH.set(_effects.bloom.lensDirtStrength);
	uExposureH.set(_effects.exposure);
	uMotionBlurH.set(GLint(_effects.motionBlur));
	uVelocityScaleH.set((float)Engine::getCurrentFps() / 60.0f);

	fullscreenTriangle->getSubMesh()->render();

	finishedTexture = fullResolutionTextureA;

	if (_effects.fxaa.enabled)
	{
		fxaa(_effects.fxaa.subPixelAA, _effects.fxaa.edgeThreshold, _effects.fxaa.edgeThresholdMin);
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

void PostProcessRenderer::fxaa(float _subPixelAA, float _edgeThreshold, float _edgeThresholdMin)
{
	glDrawBuffer((finishedTexture == fullResolutionTextureA) ? GL_COLOR_ATTACHMENT1 : GL_COLOR_ATTACHMENT0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, finishedTexture);

	fxaaShader->bind();
	uScreenTextureF.set(0);
	uSubPixelAAF.set(_subPixelAA);
	uEdgeThresholdF.set(_edgeThreshold);
	uEdgeThresholdMinF.set(_edgeThresholdMin);
	uInverseResolutionF.set(1.0f / glm::vec2(window->getWidth(), window->getHeight()));

	fullscreenTriangle->getSubMesh()->render();
	finishedTexture = (finishedTexture == fullResolutionTextureA) ? fullResolutionTextureB : fullResolutionTextureA;
}

void PostProcessRenderer::singlePassEffects(const Effects &_effects)
{
	glDrawBuffer((finishedTexture == fullResolutionTextureA) ? GL_COLOR_ATTACHMENT1 : GL_COLOR_ATTACHMENT0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, finishedTexture);

	singlePassEffectsShader->bind();
	uScreenTextureS.set(0);
	uTimeS.set((float)Engine::getCurrentTime());
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
	uColorTextureDS2.set(0);

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
	uUpscaleTextureBU.set(0);
	uPreviousBlurredTextureBU.set(1);
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
	uInputTexLFG.set(0);
	uLensColorLFG.set(1);
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
	uInputTexLFB.set(0);
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

void PostProcessRenderer::calculateCoc(GLuint _depthTexture)
{
	unsigned int width = window->getWidth();
	unsigned int height = window->getHeight();

	cocShader->bind();

	uDepthTextureCOC.set(0);
	uFocusDistanceCOC.set(50.0f);
	uFocalLengthCOC.set(30.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _depthTexture);

	glBindImageTexture(0, fullResolutionCocTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
	glDispatchCompute(width / 32 + (width % 32 ? 1 : 0), height / 32 + (height % 32 ? 1 : 0), 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void PostProcessRenderer::simpleDepthOfField(GLuint _colorTexture, GLuint _depthTexture)
{
	calculateCoc(_depthTexture);

	unsigned int width = window->getWidth();
	unsigned int height = window->getHeight();
	unsigned int halfWidth = width / 2;
	unsigned int halfHeight = height / 2;

	// blur coc
	{
		cocBlurShader->bind();
		uCocTextureCOCB.set(0);
		uDirectionCOCB.set(false);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fullResolutionCocTexture);

		glBindImageTexture(0, halfResolutionCocTexA, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
		glDispatchCompute(halfWidth / 32 + (halfWidth % 32 ? 1 : 0), halfHeight / 32 + (halfHeight % 32 ? 1 : 0), 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		glBindTexture(GL_TEXTURE_2D, halfResolutionCocTexA);

		uDirectionCOCB.set(true);

		glBindImageTexture(0, halfResolutionCocTexB, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
		glDispatchCompute(halfWidth / 32 + (halfWidth % 32 ? 1 : 0), halfHeight / 32 + (halfHeight % 32 ? 1 : 0), 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}

	static bool samplesGenerated = false;
	static bool blurSamplesSet = false;
	static bool fillSamplesSet = false;
	static glm::vec2 points64[64];
	static glm::vec2 points16[16];

	if (!samplesGenerated)
	{
		samplesGenerated = true;
		const float GOLDEN_ANGLE = 2.39996323f;

		int idx64 = 0;
		int idx16 = 0;

		for (int i = 0; i < 80; ++i)
		{
			float theta = i * GOLDEN_ANGLE;
			float r = glm::sqrt((float)i) / glm::sqrt((float)80);

			glm::vec2 p(r * glm::cos(theta), r * glm::sin(theta));

			if (i % 5 == 0)
			{
				points16[idx16] = p;
				++idx16;
			}
			else
			{
				points64[idx64] = p;
				++idx64;
			}
		}
	}

	// blur
	{
		dofBlurShader->bind();

		uColorTextureDOFB.set(0);
		uCocTextureDOFB.set(1);
		uBokehScaleDOFB.set(5.0f);
		if (!blurSamplesSet)
		{
			blurSamplesSet = true;
			for (int i = 0; i < 64; ++i)
			{
				dofBlurShader->setUniform(uSampleCoordsDOFB[i], points64[i]);
			}
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _colorTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, halfResolutionCocTexB);

		glBindImageTexture(0, halfResolutionDofTexA, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		glBindImageTexture(1, halfResolutionDofTexB, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		glDispatchCompute(halfWidth / 32 + (halfWidth % 32 ? 1 : 0), halfHeight / 32 + (halfHeight % 32 ? 1 : 0), 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}

	// fill
	{
		dofFillShader->bind();

		uColorNearTextureDOFF.set(0);
		uColorFarTextureDOFF.set(1);
		uBokehScaleDOFF.set(5.0f);
		if (!fillSamplesSet)
		{
			fillSamplesSet = true;
			for (int i = 0; i < 16; ++i)
			{
				dofFillShader->setUniform(uSampleCoordsDOFF[i], points16[i]);
			}
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, halfResolutionDofTexA);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, halfResolutionDofTexB);

		glBindImageTexture(0, halfResolutionDofTexC, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		glBindImageTexture(1, halfResolutionDofTexD, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		glDispatchCompute(halfWidth / 32 + (halfWidth % 32 ? 1 : 0), halfHeight / 32 + (halfHeight % 32 ? 1 : 0), 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		
	}
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

		glGenTextures(1, &fullResolutionCocTexture);
		glBindTexture(GL_TEXTURE_2D, fullResolutionCocTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _resolution.first, _resolution.second, 0, GL_RG, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

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

		glErrorCheck("");

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

		glErrorCheck("");
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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, _resolution.first / tileSize, _resolution.second, 0, GL_RG, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, velocityTexTmp, 0);

		glGenTextures(1, &velocityMaxTex);
		glBindTexture(GL_TEXTURE_2D, velocityMaxTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, _resolution.first / tileSize, _resolution.second / tileSize, 0, GL_RG, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenTextures(1, &velocityNeighborMaxTex);
		glBindTexture(GL_TEXTURE_2D, velocityNeighborMaxTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, _resolution.first / tileSize, _resolution.second / tileSize, 0, GL_RG, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "velocity Framebuffer not complete!" << std::endl;
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
