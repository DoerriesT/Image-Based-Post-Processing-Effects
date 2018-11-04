#include "AmbientLightRenderPass.h"
#include "Graphics\OpenGL\RenderData.h"
#include "Level.h"
#include "Graphics\Effects.h"
#include "Graphics\Texture.h"

static const char *DIRECTIONAL_LIGHT_ENABLED = "DIRECTIONAL_LIGHT_ENABLED";
static const char *SHADOWS_ENABLED = "SHADOWS_ENABLED";
static const char *SSAO_ENABLED = "SSAO_ENABLED";
static const char *GTAO_MULTI_BOUNCE_ENABLED = "GTAO_MULTI_BOUNCE_ENABLED";
static const char *IRRADIANCE_SOURCE = "IRRADIANCE_SOURCE";

AmbientLightRenderPass::AmbientLightRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	m_fbo = _fbo;
	m_drawBuffers = { GL_COLOR_ATTACHMENT4 };
	m_state.m_blendState.m_enabled = true;
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

	m_ambientLightShader = ShaderProgram::createShaderProgram(
		{ 
		{ ShaderProgram::ShaderType::FRAGMENT, DIRECTIONAL_LIGHT_ENABLED, 1 },
		{ ShaderProgram::ShaderType::FRAGMENT, SHADOWS_ENABLED, 1 },
		{ ShaderProgram::ShaderType::FRAGMENT, SSAO_ENABLED, 0 },
		{ ShaderProgram::ShaderType::FRAGMENT, GTAO_MULTI_BOUNCE_ENABLED, 0 },
		{ ShaderProgram::ShaderType::FRAGMENT, IRRADIANCE_SOURCE, 1 },
		}, 
		"Resources/Shaders/Shared/fullscreenTriangle.vert", 
		"Resources/Shaders/Lighting/ambientLight.frag");

	createUniforms();

	m_fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

bool gtaoMultiBounce = false;

void AmbientLightRenderPass::render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const Effects &_effects, GLuint ssaoTexture, GLuint _brdfLUT, RenderPass **_previousRenderPass)
{
	m_drawBuffers[0] = _renderData.m_frame % 2 ? GL_COLOR_ATTACHMENT5 : GL_COLOR_ATTACHMENT4;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	// shader permutations
	{
		const auto curDefines = m_ambientLightShader->getDefines();

		bool directionalLightEnabled = false;
		bool shadowsEnabled = false;
		bool ssaoEnabled = false;
		bool gtaoMultiBounceEnabled = false;

		for (const auto &define : curDefines)
		{
			if (std::get<0>(define) == ShaderProgram::ShaderType::FRAGMENT)
			{
				if (std::get<1>(define) == DIRECTIONAL_LIGHT_ENABLED && std::get<2>(define))
				{
					directionalLightEnabled = true;
				}
				else if (std::get<1>(define) == SHADOWS_ENABLED && std::get<2>(define))
				{
					shadowsEnabled = true;
				}
				else if (std::get<1>(define) == SSAO_ENABLED && std::get<2>(define))
				{
					ssaoEnabled = true;
				}
				else if (std::get<1>(define) == GTAO_MULTI_BOUNCE_ENABLED && std::get<2>(define))
				{
					gtaoMultiBounceEnabled = true;
				}
			}
		}

		if (directionalLightEnabled != (!_level->m_lights.m_directionalLights.empty())
			|| shadowsEnabled != _renderData.m_shadows
			|| ssaoEnabled != (_effects.m_ambientOcclusion != AmbientOcclusion::OFF)
			|| gtaoMultiBounceEnabled != gtaoMultiBounce)
		{
			m_ambientLightShader->setDefines(
				{
				{ ShaderProgram::ShaderType::FRAGMENT, DIRECTIONAL_LIGHT_ENABLED, (!_level->m_lights.m_directionalLights.empty()) },
				{ ShaderProgram::ShaderType::FRAGMENT, SHADOWS_ENABLED, _renderData.m_shadows },
				{ ShaderProgram::ShaderType::FRAGMENT, SSAO_ENABLED, (_effects.m_ambientOcclusion != AmbientOcclusion::OFF) },
				{ ShaderProgram::ShaderType::FRAGMENT, GTAO_MULTI_BOUNCE_ENABLED, gtaoMultiBounce },
				{ ShaderProgram::ShaderType::FRAGMENT, IRRADIANCE_SOURCE, 1 },
				}
			);
			createUniforms();
		}
	}

	m_fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, ssaoTexture);
	glActiveTexture(GL_TEXTURE12);
	glBindTexture(GL_TEXTURE_2D, _level->m_environment.m_irradianceVolume->getProbeTexture()->getId());
	glActiveTexture(GL_TEXTURE13);
	glBindTexture(GL_TEXTURE_2D, _level->m_environment.m_environmentProbes[0]->getReflectionTexture()->getId());
	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, _brdfLUT);
	//glActiveTexture(GL_TEXTURE10);
	//glBindTexture(GL_TEXTURE_2D, 0); // TODO: remove and redo SSR; this is previous frame's texture

	//glActiveTexture(GL_TEXTURE5);
	//glBindTexture(GL_TEXTURE_2D, _lpv[0]);
	//glActiveTexture(GL_TEXTURE6);
	//glBindTexture(GL_TEXTURE_2D, _lpv[1]);
	//glActiveTexture(GL_TEXTURE7);
	//glBindTexture(GL_TEXTURE_2D, _lpv[2]);

	m_ambientLightShader->bind();

	if (!_level->m_lights.m_directionalLights.empty())
	{
		const auto light = _level->m_lights.m_directionalLights[0];
		light->updateViewValues(_renderData.m_viewMatrix);
		if (light->isRenderShadows())
		{
			glActiveTexture(GL_TEXTURE15);
			glBindTexture(GL_TEXTURE_2D_ARRAY, light->getShadowMap());
		}
		m_uDirectionalLight.set(light);
	}

	m_uOddFrame.set(_renderData.m_frame & 1);
	m_uInverseView.set(_renderData.m_invViewMatrix);
	m_uInverseProjection.set(_renderData.m_invProjectionMatrix);

	//if (_effects.diffuseAmbientSource == DiffuseAmbientSource::LIGHT_PROPAGATION_VOLUMES)
	//{
	//	uVolumeOrigin.set(_volume.origin);
	//	uVolumeDimensions.set(_volume.dimensions);
	//	uSpacing.set(_volume.spacing);
	//}
	//else
	//{
		std::shared_ptr<IrradianceVolume> volume = _level->m_environment.m_irradianceVolume;
		m_uVolumeOrigin.set(volume->getOrigin());
		m_uVolumeDimensions.set(volume->getDimensions());
		m_uSpacing.set(volume->getSpacing());
	//}

	m_fullscreenTriangle->getSubMesh()->render();
}

void AmbientLightRenderPass::createUniforms()
{
	m_uDirectionalLight.create(m_ambientLightShader);

	m_uOddFrame.create(m_ambientLightShader);
	m_uInverseProjection.create(m_ambientLightShader);
	m_uInverseView.create(m_ambientLightShader);

	m_uVolumeOrigin.create(m_ambientLightShader);
	m_uVolumeDimensions.create(m_ambientLightShader);
	m_uSpacing.create(m_ambientLightShader);
}
