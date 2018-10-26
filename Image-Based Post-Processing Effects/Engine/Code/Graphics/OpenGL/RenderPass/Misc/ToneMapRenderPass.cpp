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
	m_fbo = _fbo;
	m_drawBuffers = { GL_COLOR_ATTACHMENT0 };
	m_state.m_blendState.m_enabled = false;
	m_state.m_blendState.m_sFactor = GL_ONE;
	m_state.m_blendState.m_dFactor = GL_ONE;
	m_state.m_cullFaceState.m_enabled = false;
	m_state.m_cullFaceState.m_face = GL_BACK;
	m_state.m_depthState.m_enabled = false;
	m_state.m_depthState.m_func = GL_LEQUAL;
	m_state.m_depthState.m_mask = GL_FALSE;
	m_state.m_stencilState.m_enabled = false;
	m_state.m_stencilState.m_frontFunc = m_state.m_stencilState.m_backFunc = GL_ALWAYS;
	m_state.m_stencilState.m_frontRef = m_state.m_stencilState.m_backRef = 1;
	m_state.m_stencilState.m_frontMask = m_state.m_stencilState.m_backMask = 0xFF;
	m_state.m_stencilState.m_frontOpFail = m_state.m_stencilState.m_backOpFail = GL_KEEP;
	m_state.m_stencilState.m_frontOpZfail = m_state.m_stencilState.m_backOpZfail = GL_KEEP;
	m_state.m_stencilState.m_frontOpZpass = m_state.m_stencilState.m_backOpZpass = GL_KEEP;

	resize(_width, _height);

	m_toneMapShader = ShaderProgram::createShaderProgram(
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

	m_uStarburstOffset.create(m_toneMapShader);
	m_uBloomStrength.create(m_toneMapShader);
	m_uLensDirtStrength.create(m_toneMapShader);
	m_uExposure.create(m_toneMapShader);
	m_uAnamorphicFlareColor.create(m_toneMapShader);

	m_fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void ToneMapRenderPass::render(const Effects &_effects, float _starburstOffset, RenderPass ** _previousRenderPass)
{
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	static std::shared_ptr<Texture> lensDirtTexture = Texture::createTexture("Resources/Textures/lensdirt.dds", true);
	static std::shared_ptr<Texture> lensStarTexture = Texture::createTexture("Resources/Textures/starburst.dds", true);

	m_fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	// shader permutations
	{
		const auto curDefines = m_toneMapShader->getDefines();
	
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
	
		if (bloomEnabled != _effects.m_bloom.m_enabled
			|| flaresEnabled != _effects.m_lensFlares.m_enabled
			|| dirtEnabled != _effects.m_lensDirt.m_enabled
			|| godRaysEnabled != _effects.m_godrays
			|| autoExposureEnabled != true
			|| anamorphicsFlaresEnabled != _effects.m_anamorphicFlares.m_enabled
			|| motionBlur != int(_effects.m_motionBlur))
		{
			m_toneMapShader->setDefines(
				{
				{ ShaderProgram::ShaderType::FRAGMENT, BLOOM_ENABLED, _effects.m_bloom.m_enabled },
				{ ShaderProgram::ShaderType::FRAGMENT, FLARES_ENABLED, _effects.m_lensFlares.m_enabled },
				{ ShaderProgram::ShaderType::FRAGMENT, ANAMORPHIC_FLARES_ENABLED, _effects.m_anamorphicFlares.m_enabled },
				{ ShaderProgram::ShaderType::FRAGMENT, DIRT_ENABLED, _effects.m_lensDirt.m_enabled },
				{ ShaderProgram::ShaderType::FRAGMENT, GOD_RAYS_ENABLED, _effects.m_godrays },
				{ ShaderProgram::ShaderType::FRAGMENT, AUTO_EXPOSURE_ENABLED, 1 },
				{ ShaderProgram::ShaderType::FRAGMENT, MOTION_BLUR, int(_effects.m_motionBlur) },
				}
			);
			m_uStarburstOffset.create(m_toneMapShader);
			m_uBloomStrength.create(m_toneMapShader);
			m_uLensDirtStrength.create(m_toneMapShader);
			m_uExposure.create(m_toneMapShader);
			m_uAnamorphicFlareColor.create(m_toneMapShader);
		}
	}

	m_toneMapShader->bind();

	m_uStarburstOffset.set(_starburstOffset);
	m_uBloomStrength.set(_effects.m_bloom.m_strength);
	m_uExposure.set(_effects.m_exposure);
	m_uLensDirtStrength.set(_effects.m_lensDirt.m_strength);
	m_uAnamorphicFlareColor.set(_effects.m_anamorphicFlares.m_color);


	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, lensDirtTexture->getId());
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, lensStarTexture->getId());

	m_fullscreenTriangle->getSubMesh()->render();
}
