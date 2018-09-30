#include "ToneMapRenderPass.h"
#include "Graphics\Texture.h"
#include "Graphics\Effects.h"

static const char *BLOOM_ENABLED = "BLOOM_ENABLED";
static const char *FLARES_ENABLED = "FLARES_ENABLED";
static const char *DIRT_ENABLED = "DIRT_ENABLED";
static const char *GOD_RAYS_ENABLED = "GOD_RAYS_ENABLED";
static const char *AUTO_EXPOSURE_ENABLED = "AUTO_EXPOSURE_ENABLED";
static const char *MOTION_BLUR = "MOTION_BLUR";
static const char *ANAMORPHIC_FLARES_ENABLED = "ANAMORPHIC_FLARES_ENABLED";

ToneMapRenderPass::ToneMapRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	fbo = _fbo;
	drawBuffers = { GL_COLOR_ATTACHMENT0 };
	state.blendState.enabled = false;
	state.blendState.sFactor = GL_ONE;
	state.blendState.dFactor = GL_ONE;
	state.cullFaceState.enabled = false;
	state.cullFaceState.face = GL_BACK;
	state.depthState.enabled = false;
	state.depthState.func = GL_LEQUAL;
	state.depthState.mask = GL_FALSE;
	state.stencilState.enabled = false;
	state.stencilState.frontFunc = state.stencilState.backFunc = GL_ALWAYS;
	state.stencilState.frontRef = state.stencilState.backRef = 1;
	state.stencilState.frontMask = state.stencilState.backMask = 0xFF;
	state.stencilState.frontOpFail = state.stencilState.backOpFail = GL_KEEP;
	state.stencilState.frontOpZfail = state.stencilState.backOpZfail = GL_KEEP;
	state.stencilState.frontOpZpass = state.stencilState.backOpZpass = GL_KEEP;

	resize(_width, _height);

	toneMapShader = ShaderProgram::createShaderProgram(
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

	uStarburstOffsetH.create(toneMapShader);
	uBloomStrengthH.create(toneMapShader);
	uLensDirtStrengthH.create(toneMapShader);
	uExposureH.create(toneMapShader);
	uAnamorphicFlareColorH.create(toneMapShader);

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void ToneMapRenderPass::render(const Effects &_effects, float _starburstOffset, RenderPass ** _previousRenderPass)
{
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	static std::shared_ptr<Texture> lensDirtTexture = Texture::createTexture("Resources/Textures/lensdirt.dds", true);
	static std::shared_ptr<Texture> lensStarTexture = Texture::createTexture("Resources/Textures/starburst.dds", true);

	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	// shader permutations
	{
		const auto curDefines = toneMapShader->getDefines();
	
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
			|| godRaysEnabled != _effects.godrays
			|| autoExposureEnabled != true
			|| anamorphicsFlaresEnabled != _effects.anamorphicFlares.enabled
			|| motionBlur != int(_effects.motionBlur))
		{
			toneMapShader->setDefines(
				{
				{ ShaderProgram::ShaderType::FRAGMENT, BLOOM_ENABLED, _effects.bloom.enabled },
				{ ShaderProgram::ShaderType::FRAGMENT, FLARES_ENABLED, _effects.lensFlares.enabled },
				{ ShaderProgram::ShaderType::FRAGMENT, ANAMORPHIC_FLARES_ENABLED, _effects.anamorphicFlares.enabled },
				{ ShaderProgram::ShaderType::FRAGMENT, DIRT_ENABLED, _effects.lensDirt.enabled },
				{ ShaderProgram::ShaderType::FRAGMENT, GOD_RAYS_ENABLED, _effects.godrays },
				{ ShaderProgram::ShaderType::FRAGMENT, AUTO_EXPOSURE_ENABLED, 1 },
				{ ShaderProgram::ShaderType::FRAGMENT, MOTION_BLUR, int(_effects.motionBlur) },
				}
			);
			uStarburstOffsetH.create(toneMapShader);
			uBloomStrengthH.create(toneMapShader);
			uLensDirtStrengthH.create(toneMapShader);
			uExposureH.create(toneMapShader);
			uAnamorphicFlareColorH.create(toneMapShader);
		}
	}

	toneMapShader->bind();

	uStarburstOffsetH.set(_starburstOffset);
	uBloomStrengthH.set(_effects.bloom.strength);
	uExposureH.set(_effects.exposure);
	uLensDirtStrengthH.set(_effects.lensDirt.strength);
	uAnamorphicFlareColorH.set(_effects.anamorphicFlares.color);


	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, lensDirtTexture->getId());
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, lensStarTexture->getId());

	fullscreenTriangle->getSubMesh()->render();
}
