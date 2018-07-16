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
	lensFlareGenShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/PostProcess/LensFlares/lensFlareGen.frag");
	lensFlareBlurShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/PostProcess/LensFlares/lensFlareBlur.frag");
	downsampleShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/PostProcess/downsample.frag");
	upsampleShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/PostProcess/upsample.frag");
	velocityTileMaxShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/PostProcess/MotionBlur/velocityTileMax.frag");
	velocityNeighborTileMaxShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/PostProcess/MotionBlur/velocityNeighborTileMax.frag");
	cocTileMaxShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/PostProcess/DepthOfField/cocTileMax.frag");
	cocNeighborTileMaxShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/PostProcess/DepthOfField/cocNeighborTileMax.frag");
	cocShader = ShaderProgram::createShaderProgram("Resources/Shaders/PostProcess/DepthOfField/coc.comp");
	cocBlurShader = ShaderProgram::createShaderProgram("Resources/Shaders/PostProcess/DepthOfField/cocBlur.comp");
	dofBlurShader = ShaderProgram::createShaderProgram("Resources/Shaders/PostProcess/DepthOfField/Simple/dofBlur.comp");
	dofFillShader = ShaderProgram::createShaderProgram("Resources/Shaders/PostProcess/DepthOfField/Simple/dofFill.comp");
	dofCompositeShader = ShaderProgram::createShaderProgram("Resources/Shaders/PostProcess/DepthOfField/Simple/dofComposite.comp");
	dofSeperateBlurShader = ShaderProgram::createShaderProgram("Resources/Shaders/PostProcess/DepthOfField/TileBasedSeperate/dofSeperatedBlur.comp");
	dofSeperateFillShader = ShaderProgram::createShaderProgram("Resources/Shaders/PostProcess/DepthOfField/TileBasedSeperate/dofSeperatedFill.comp");
	dofSeperateCompositeShader = ShaderProgram::createShaderProgram("Resources/Shaders/PostProcess/DepthOfField/TileBasedSeperate/dofSeperatedComposite.comp");
	dofCombinedBlurShader = ShaderProgram::createShaderProgram("Resources/Shaders/PostProcess/DepthOfField/TileBasedCombined/dofCombinedBlur.comp");
	dofSpriteShader = ShaderProgram::createShaderProgram("Resources/Shaders/PostProcess/DepthOfField/SpriteBased/dofSprite.vert", "Resources/Shaders/PostProcess/DepthOfField/SpriteBased/dofSprite.frag");
	dofSpriteComposeShader = ShaderProgram::createShaderProgram("Resources/Shaders/PostProcess/DepthOfField/SpriteBased/dofSpriteCompose.comp");
	luminanceGenShader = ShaderProgram::createShaderProgram("Resources/Shaders/PostProcess/luminanceGen.comp");
	luminanceAdaptionShader = ShaderProgram::createShaderProgram("Resources/Shaders/PostProcess/luminanceAdaption.comp");
	godRayMaskShader = ShaderProgram::createShaderProgram("Resources/Shaders/PostProcess/GodRays/godRayMask.comp");
	godRayGenShader = ShaderProgram::createShaderProgram("Resources/Shaders/PostProcess/GodRays/godRayGen.comp");

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
	uLensFlaresH.create(hdrShader);
	uBloomH.create(hdrShader);
	uBloomStrengthH.create(hdrShader);
	uBloomDirtStrengthH.create(hdrShader);
	uExposureH.create(hdrShader);
	uMotionBlurH.create(hdrShader);
	uGodRaysH.create(hdrShader);

	// fxaa
	uInverseResolutionF.create(fxaaShader);
	uSubPixelAAF.create(fxaaShader);
	uEdgeThresholdF.create(fxaaShader);
	uEdgeThresholdMinF.create(fxaaShader);

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

	// velocity tile max
	uDirectionVTM.create(velocityTileMaxShader);
	uTileSizeVTM.create(velocityTileMaxShader);

	// coc
	uFocalLengthCOC.create(cocShader);
	uApertureSizeCOC.create(cocShader);
	uNearFarCOC.create(cocShader);

	// coc blur
	uDirectionCOCB.create(cocBlurShader);

	// coc tile max
	uDirectionCOCTM.create(cocTileMaxShader);
	uTileSizeCOCTM.create(cocTileMaxShader);

	// dof blur
	for (int i = 0; i < 7 * 7; ++i)
	{
		uSampleCoordsDOFB.push_back(dofBlurShader->createUniform(std::string("uSampleCoords") + "[" + std::to_string(i) + "]"));
	}

	// dof fill
	for (int i = 0; i < 3 * 3; ++i)
	{
		uSampleCoordsDOFF.push_back(dofFillShader->createUniform(std::string("uSampleCoords") + "[" + std::to_string(i) + "]"));
	}

	// dof seperate blur
	for (int i = 0; i < 7 * 7; ++i)
	{
		uSampleCoordsSDOFB.push_back(dofSeperateBlurShader->createUniform(std::string("uSampleCoords") + "[" + std::to_string(i) + "]"));
	}

	// dof seperate fill
	for (int i = 0; i < 3 * 3; ++i)
	{
		uSampleCoordsSDOFF.push_back(dofSeperateFillShader->createUniform(std::string("uSampleCoords") + "[" + std::to_string(i) + "]"));
	}

	// dof combined blur
	for (int i = 0; i < 7 * 7; ++i)
	{
		uSampleCoordsCDOFB.push_back(dofCombinedBlurShader->createUniform(std::string("uSampleCoords") + "[" + std::to_string(i) + "]"));
	}

	// dof sprite
	uWidthDOF.create(dofSpriteShader);
	uHeightDOF.create(dofSpriteShader);

	// luminance adaption
	uTimeDeltaLA.create(luminanceAdaptionShader);
	uTauLA.create(luminanceAdaptionShader);

	// god ray gen
	uSunPosGR.create(godRayGenShader);

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
	createFboAttachments(std::make_pair(window->getWidth(), window->getHeight()));

	// load textures
	lensColorTexture = Texture::createTexture("Resources/Textures/lenscolor.dds", true);
	lensDirtTexture = Texture::createTexture("Resources/Textures/lensdirt.dds", true);
	lensStarTexture = Texture::createTexture("Resources/Textures/starburst.dds", true);

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);


	// sprite

	glm::vec2 spritePositions[] = 
	{
		glm::vec2(-1.0, -1.0),
		glm::vec2(1.0, -1.0),
		glm::vec2(-1.0, 1.0),
		glm::vec2(1.0, 1.0)
	};

	uint32_t spriteIndices[] =
	{
		0, 1, 2, 1, 3, 2
	};

	// create buffers/arrays
	glGenVertexArrays(1, &spriteVAO);
	glGenBuffers(1, &spriteVBO);
	glGenBuffers(1, &spriteEBO);
	glBindVertexArray(spriteVAO);
	glBindBuffer(GL_ARRAY_BUFFER, spriteVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(spritePositions), spritePositions, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, spriteEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(spriteIndices), spriteIndices, GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);

	glBindVertexArray(0);
}

unsigned int tileSize = 40;
bool godrays = true;

void PostProcessRenderer::render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const Effects &_effects, GLuint _colorTexture, GLuint _depthTexture, GLuint _velocityTexture, const std::shared_ptr<Camera> &_camera)
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

			glBindTexture(GL_TEXTURE_2D, velocityMaxTex);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, velocityNeighborMaxTex, 0);

			fullscreenTriangle->getSubMesh()->render();
		}

	}

	uGodRaysH.set(godrays);
	if (godrays && !_level->lights.directionalLights.empty())
	{
		glm::vec2 sunpos = glm::vec2(_renderData.viewProjectionMatrix * glm::vec4(_level->lights.directionalLights[0]->getDirection(), 0.0f)) * 0.5f + 0.5f;
		godRays(sunpos, _colorTexture, _depthTexture);
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
		tileBasedCombinedFieldDepthOfField(_colorTexture, _depthTexture);
		break;
	default:
		break;
	}

	calculateLuminance(_colorTexture);

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

	hdrShader->bind();

	uStarburstOffsetH.set(glm::dot(glm::vec3(1.0), _camera->getForwardDirection()));
	uLensFlaresH.set(_effects.lensFlares.enabled);
	uBloomH.set(_effects.bloom.enabled);
	uBloomStrengthH.set(_effects.bloom.strength);
	uBloomDirtStrengthH.set(_effects.bloom.lensDirtStrength);
	uExposureH.set(_effects.exposure);
	uMotionBlurH.set(GLint(_effects.motionBlur));

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

void PostProcessRenderer::calculateCoc(GLuint _depthTexture)
{
	unsigned int width = window->getWidth();
	unsigned int height = window->getHeight();

	cocShader->bind();

	const float filmWidth = 35.0f;
	const float apertureSize = 8.0f;

	float focalLength = (0.5f * filmWidth) / glm::tan(window->getFieldOfView() * 0.5f);

	uFocalLengthCOC.set(focalLength);
	uApertureSizeCOC.set(8.0f);
	uNearFarCOC.set(glm::vec2(Window::NEAR_PLANE, Window::FAR_PLANE));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _depthTexture);

	glBindImageTexture(0, fullResolutionCocTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
	GLUtility::glDispatchComputeHelper(width, height, 1, 8, 8, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void PostProcessRenderer::calculateCocTileTexture()
{
	glBindFramebuffer(GL_FRAMEBUFFER, cocFbo);
	glActiveTexture(GL_TEXTURE0);

	// tile max
	{
		cocTileMaxShader->bind();

		uTileSizeCOCTM.set(tileSize);

		// fullscreen to first step
		{
			glViewport(0, 0, window->getWidth() / tileSize, window->getHeight());
			glBindTexture(GL_TEXTURE_2D, fullResolutionCocTexture);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cocTexTmp, 0);

			uDirectionCOCTM.set(false);

			fullscreenTriangle->getSubMesh()->render();
		}

		// first to second step
		{
			glViewport(0, 0, window->getWidth() / tileSize, window->getHeight() / tileSize);
			glBindTexture(GL_TEXTURE_2D, cocTexTmp);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cocMaxTex, 0);

			uDirectionCOCTM.set(true);

			fullscreenTriangle->getSubMesh()->render();
		}
	}

	// tile neighbor max
	{
		cocNeighborTileMaxShader->bind();

		glBindTexture(GL_TEXTURE_2D, cocMaxTex);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cocNeighborMaxTex, 0);

		fullscreenTriangle->getSubMesh()->render();
	}
}

float PI = glm::pi<float>();

glm::vec2 generateDepthOfFieldSample(const glm::vec2 &_origin)
{
	float max_fstops = 8;
	float min_fstops = 1;
	float normalizedStops = 1.0f; //clamp_tpl((fstop - max_fstops) / (max_fstops - min_fstops), 0.0f, 1.0f);

	float phi;
	float r;
	const float a = 2 * _origin.x - 1;
	const float b = 2 * _origin.y - 1;
	if (abs(a) > abs(b)) // Use squares instead of absolute values
	{
		r = a;
		phi = (PI / 4.0f) * (b / (a + 1e-6f));
	}
	else
	{
		r = b;
		phi = (PI / 2.0f) - (PI / 4.0f) * (a / (b + 1e-6f));
	}

	float rr = r;
	rr = abs(rr) * (rr > 0.0f ? 1.0f : -1.0f);

	//normalizedStops *= -0.4f * PI;
	return glm::vec2(rr * cosf(phi + normalizedStops), rr * sinf(phi + normalizedStops));
}

void PostProcessRenderer::simpleDepthOfField(GLuint _colorTexture, GLuint _depthTexture)
{
	unsigned int width = window->getWidth();
	unsigned int height = window->getHeight();
	unsigned int halfWidth = width / 2;
	unsigned int halfHeight = height / 2;

	const float filmWidth = 35.0f;
	const float focalLength = (0.5f * filmWidth) / glm::tan(window->getFieldOfView() * 0.5f);

	// blur coc
	{
		cocBlurShader->bind();
		uDirectionCOCB.set(false);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fullResolutionCocTexture);

		glBindImageTexture(0, halfResolutionCocTexA, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
		GLUtility::glDispatchComputeHelper(halfWidth, halfHeight, 1, 8, 8, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		glBindTexture(GL_TEXTURE_2D, halfResolutionCocTexA);

		uDirectionCOCB.set(true);

		glBindImageTexture(0, halfResolutionCocTexB, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
		GLUtility::glDispatchComputeHelper(halfWidth, halfHeight, 1, 8, 8, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}

	static bool samplesGenerated = false;
	static bool blurSamplesSet = false;
	static bool fillSamplesSet = false;
	static glm::vec2 blurSamples[7 * 7];
	static glm::vec2 fillSamples[3 * 3];

	if (!samplesGenerated)
	{
		samplesGenerated = true;

		unsigned int nSquareTapsSide = 7;
		float fRecipTaps = 1.0f / ((float)nSquareTapsSide - 1.0f);

		for (unsigned int y = 0; y < nSquareTapsSide; ++y)
		{
			for (unsigned int x = 0; x < nSquareTapsSide; ++x)
			{
				blurSamples[y * nSquareTapsSide + x] = generateDepthOfFieldSample(glm::vec2(x * fRecipTaps, y * fRecipTaps));
			}
		}

		nSquareTapsSide = 3;
		fRecipTaps = 1.0f / ((float)nSquareTapsSide - 1.0f);

		for (unsigned int y = 0; y < nSquareTapsSide; ++y)
		{
			for (unsigned int x = 0; x < nSquareTapsSide; ++x)
			{
				fillSamples[y * nSquareTapsSide + x] = generateDepthOfFieldSample(glm::vec2(x * fRecipTaps, y * fRecipTaps));
			}
		}
	}

	// blur
	{
		dofBlurShader->bind();

		if (!blurSamplesSet)
		{
			blurSamplesSet = true;
			for (int i = 0; i < 7 * 7; ++i)
			{
				dofBlurShader->setUniform(uSampleCoordsDOFB[i], blurSamples[i]);
			}
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _colorTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, halfResolutionCocTexB);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, halfResolutionDofTexA);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, halfResolutionDofTexB);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, halfResolutionDofTexC);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, halfResolutionDofTexD);

		glBindImageTexture(0, halfResolutionDofTexA, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		glBindImageTexture(1, halfResolutionDofTexB, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		GLUtility::glDispatchComputeHelper(halfWidth, halfHeight, 1, 8, 8, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}

	// fill
	{
		dofFillShader->bind();

		if (!fillSamplesSet)
		{
			fillSamplesSet = true;
			for (int i = 0; i < 3 * 3; ++i)
			{
				dofFillShader->setUniform(uSampleCoordsDOFF[i], fillSamples[i]);
			}
		}

		glBindImageTexture(0, halfResolutionDofTexC, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		glBindImageTexture(1, halfResolutionDofTexD, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		GLUtility::glDispatchComputeHelper(halfWidth, halfHeight, 1, 8, 8, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	}

	// composite
	{
		dofCompositeShader->bind();

		glBindImageTexture(0, fullResolutionHdrTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		GLUtility::glDispatchComputeHelper(width, height, 1, 8, 8, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
}

void PostProcessRenderer::tileBasedSeperateFieldDepthOfField(GLuint _colorTexture)
{
	calculateCocTileTexture();

	unsigned int width = window->getWidth();
	unsigned int height = window->getHeight();
	unsigned int halfWidth = width / 2;
	unsigned int halfHeight = height / 2;

	const float filmWidth = 35.0f;
	const float apertureSize = 8.0f;
	const float focalLength = (0.5f * filmWidth) / glm::tan(window->getFieldOfView() * 0.5f);
	const float blades = 6.0f;


	static bool samplesGenerated = false;
	static bool blurSamplesSet = false;
	static bool fillSamplesSet = false;
	static glm::vec2 blurSamples[7 * 7];
	static glm::vec2 fillSamples[3 * 3];

	if (!samplesGenerated)
	{
		samplesGenerated = true;

		unsigned int nSquareTapsSide = 7;
		float fRecipTaps = 1.0f / ((float)nSquareTapsSide - 1.0f);

		for (unsigned int y = 0; y < nSquareTapsSide; ++y)
		{
			for (unsigned int x = 0; x < nSquareTapsSide; ++x)
			{
				blurSamples[y * nSquareTapsSide + x] = generateDepthOfFieldSample(glm::vec2(x * fRecipTaps, y * fRecipTaps));
			}
		}

		nSquareTapsSide = 3;
		fRecipTaps = 1.0f / ((float)nSquareTapsSide - 1.0f);
		const float rotAngle = glm::radians(15.0f);
		const glm::mat2 rot = glm::mat2(glm::cos(rotAngle), -glm::sin(rotAngle), glm::sin(rotAngle), glm::cos(rotAngle));

		for (unsigned int y = 0; y < nSquareTapsSide; ++y)
		{
			for (unsigned int x = 0; x < nSquareTapsSide; ++x)
			{
				fillSamples[y * nSquareTapsSide + x] = rot * generateDepthOfFieldSample(glm::vec2(x * fRecipTaps, y * fRecipTaps));
			}
		}
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _colorTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fullResolutionCocTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, cocNeighborMaxTex);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, fullResolutionDofTexA);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, fullResolutionDofTexB);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, fullResolutionDofTexC);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, fullResolutionDofTexD);

	// blur
	{
		dofSeperateBlurShader->bind();

		if (!blurSamplesSet)
		{
			blurSamplesSet = true;
			for (int i = 0; i < 7 * 7; ++i)
			{
				dofSeperateBlurShader->setUniform(uSampleCoordsSDOFB[i], blurSamples[i]);
			}
		}

		glBindImageTexture(0, fullResolutionDofTexA, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		glBindImageTexture(1, fullResolutionDofTexB, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		GLUtility::glDispatchComputeHelper(width, height, 1, 8, 8, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}

	// fill
	{
		dofSeperateFillShader->bind();

		if (!fillSamplesSet)
		{
			fillSamplesSet = true;
			for (int i = 0; i < 3 * 3; ++i)
			{
				dofSeperateFillShader->setUniform(uSampleCoordsSDOFF[i], fillSamples[i]);
			}
		}

		glBindImageTexture(0, fullResolutionDofTexC, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		glBindImageTexture(1, fullResolutionDofTexD, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		GLUtility::glDispatchComputeHelper(width, height, 1, 8, 8, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	}

	// composite
	{
		dofSeperateCompositeShader->bind();

		glBindImageTexture(0, fullResolutionHdrTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		GLUtility::glDispatchComputeHelper(width, height, 1, 8, 8, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
}

void PostProcessRenderer::tileBasedCombinedFieldDepthOfField(GLuint _colorTexture, GLuint _depthTexture)
{
	calculateCocTileTexture();

	unsigned int width = window->getWidth();
	unsigned int height = window->getHeight();
	unsigned int halfWidth = width / 2;
	unsigned int halfHeight = height / 2;

	const float filmWidth = 35.0f;
	const float apertureSize = 8.0f;
	const float focalLength = (0.5f * filmWidth) / glm::tan(window->getFieldOfView() * 0.5f);
	const float blades = 6.0f;


	static bool samplesGenerated = false;
	static bool blurSamplesSet = false;
	static bool fillSamplesSet = false;
	static glm::vec2 blurSamples[7 * 7];
	static glm::vec2 fillSamples[3 * 3];

	if (!samplesGenerated)
	{
		samplesGenerated = true;

		unsigned int nSquareTapsSide = 7;
		float fRecipTaps = 1.0f / ((float)nSquareTapsSide - 1.0f);

		for (unsigned int y = 0; y < nSquareTapsSide; ++y)
		{
			for (unsigned int x = 0; x < nSquareTapsSide; ++x)
			{
				blurSamples[y * nSquareTapsSide + x] = generateDepthOfFieldSample(glm::vec2(x * fRecipTaps, y * fRecipTaps));
			}
		}

		nSquareTapsSide = 3;
		fRecipTaps = 1.0f / ((float)nSquareTapsSide - 1.0f);
		const float rotAngle = glm::radians(15.0f);
		const glm::mat2 rot = glm::mat2(glm::cos(rotAngle), -glm::sin(rotAngle), glm::sin(rotAngle), glm::cos(rotAngle));

		for (unsigned int y = 0; y < nSquareTapsSide; ++y)
		{
			for (unsigned int x = 0; x < nSquareTapsSide; ++x)
			{
				fillSamples[y * nSquareTapsSide + x] = rot * generateDepthOfFieldSample(glm::vec2(x * fRecipTaps, y * fRecipTaps));
			}
		}
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _colorTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fullResolutionCocTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, cocNeighborMaxTex);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, _depthTexture);

	// blur
	{
		dofCombinedBlurShader->bind();

		if (!blurSamplesSet)
		{
			blurSamplesSet = true;
			for (int i = 0; i < 7 * 7; ++i)
			{
				dofCombinedBlurShader->setUniform(uSampleCoordsCDOFB[i], blurSamples[i]);
			}
		}

		glBindImageTexture(0, fullResolutionHdrTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		GLUtility::glDispatchComputeHelper(width, height, 1, 8, 8, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
}

void PostProcessRenderer::spriteBasedDepthOfField(GLuint _colorTexture, GLuint _depthTexture)
{
	static std::shared_ptr<Texture> bokehSprite = Texture::createTexture("Resources/Textures/bokehsprite.dds", true);
	unsigned int width = window->getWidth();
	unsigned int height = window->getHeight();
	unsigned int halfWidth = width / 2;
	unsigned int halfHeight = height / 2;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fullResolutionCocTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _colorTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, _depthTexture);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, halfResolutionDofDoubleTex);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, bokehSprite->getId());

	glBindFramebuffer(GL_FRAMEBUFFER, cocFbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, halfResolutionDofDoubleTex, 0);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glViewport(0, 0, width, halfHeight);

	glBindVertexArray(spriteVAO);
	glEnableVertexAttribArray(0);

	dofSpriteShader->bind();

	uWidthDOF.set(halfWidth);
	uHeightDOF.set(halfHeight);

	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL, halfWidth * halfHeight);

	glDisable(GL_BLEND);

	dofSpriteComposeShader->bind();

	glBindImageTexture(0, fullResolutionHdrTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	GLUtility::glDispatchComputeHelper(width, height, 1, 8, 8, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void PostProcessRenderer::godRays(const glm::vec2 &_sunpos, GLuint _colorTexture, GLuint _depthTexture)
{
	unsigned int width = window->getWidth();
	unsigned int height = window->getHeight();
	unsigned int halfWidth = width / 2;
	unsigned int halfHeight = height / 2;

	// mask
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _depthTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, _colorTexture);

		godRayMaskShader->bind();

		glBindImageTexture(0, halfResolutionGodRayTexA, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		GLUtility::glDispatchComputeHelper(halfWidth, halfHeight, 1, 8, 8, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
	
	// god ray gen
	{
		godRayGenShader->bind();
		uSunPosGR.set(_sunpos);
	
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, halfResolutionGodRayTexA);
	
		glBindImageTexture(0, halfResolutionGodRayTexB, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		GLUtility::glDispatchComputeHelper(halfWidth, halfHeight, 1, 8, 8, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, halfResolutionGodRayTexB);
		
		glBindImageTexture(0, halfResolutionGodRayTexA, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		GLUtility::glDispatchComputeHelper(halfWidth, halfHeight, 1, 8, 8, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, halfResolutionGodRayTexA);
		
		glBindImageTexture(0, halfResolutionGodRayTexB, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		GLUtility::glDispatchComputeHelper(halfWidth, halfHeight, 1, 8, 8, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
}

void PostProcessRenderer::calculateLuminance(GLuint _colorTexture)
{
	currentLuminanceTexture = !currentLuminanceTexture;

	luminanceGenShader->bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _colorTexture);

	glBindImageTexture(0, luminanceTempTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16F);
	glDispatchCompute(1024 / 8, 1024 / 8, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glBindTexture(GL_TEXTURE_2D, luminanceTempTexture);
	glGenerateMipmap(GL_TEXTURE_2D);

	luminanceAdaptionShader->bind();
	uTimeDeltaLA.set((float)Engine::getTimeDelta());
	uTauLA.set(2.5f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, luminanceTexture[!currentLuminanceTexture]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, luminanceTempTexture);

	glBindImageTexture(0, luminanceTexture[currentLuminanceTexture], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16F);
	glDispatchCompute(1, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
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

	// coc
	{
		glBindFramebuffer(GL_FRAMEBUFFER, cocFbo);

		glGenTextures(1, &cocTexTmp);
		glBindTexture(GL_TEXTURE_2D, cocTexTmp);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _resolution.first / tileSize, _resolution.second, 0, GL_RG, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cocTexTmp, 0);

		glGenTextures(1, &cocMaxTex);
		glBindTexture(GL_TEXTURE_2D, cocMaxTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _resolution.first / tileSize, _resolution.second / tileSize, 0, GL_RG, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenTextures(1, &cocNeighborMaxTex);
		glBindTexture(GL_TEXTURE_2D, cocNeighborMaxTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, _resolution.first / tileSize, _resolution.second / tileSize, 0, GL_RG, GL_FLOAT, NULL);
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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, 1024, 1024, 0, GL_RED, GL_FLOAT, NULL);
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

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
