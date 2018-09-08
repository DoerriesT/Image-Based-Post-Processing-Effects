#include "AmbientLightRenderPass.h"
#include "Graphics\OpenGL\RenderData.h"
#include "Level.h"
#include "Graphics\Effects.h"
#include "Graphics\Texture.h"

static const char *SSAO_ENABLED = "SSAO_ENABLED";
static const char *SSR_ENABLED = "SSR_ENABLED";
static const char *IRRADIANCE_VOLUME = "IRRADIANCE_VOLUME";

AmbientLightRenderPass::AmbientLightRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	fbo = _fbo;
	drawBuffers = { GL_COLOR_ATTACHMENT4 };
	state.blendState.enabled = true;
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

	environmentLightPassShader = ShaderProgram::createShaderProgram(
		{ 
		{ ShaderProgram::ShaderType::FRAGMENT, SSAO_ENABLED, 0 },
		{ ShaderProgram::ShaderType::FRAGMENT, SSR_ENABLED, 0 },
		{ ShaderProgram::ShaderType::FRAGMENT, IRRADIANCE_VOLUME, 1 },
		}, 
		"Resources/Shaders/Shared/fullscreenTriangle.vert", 
		"Resources/Shaders/Lighting/ambientLightPass.frag");

	createUniforms();

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

bool flatAmbient;

void AmbientLightRenderPass::render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const Effects &_effects, const GBuffer &_gbuffer, GLuint _brdfLUT, RenderPass **_previousRenderPass)
{
	drawBuffers[0] = _renderData.frame % 2 ? GL_COLOR_ATTACHMENT5 : GL_COLOR_ATTACHMENT4;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	// shader permutations
	{
		const auto curDefines = environmentLightPassShader->getDefines();

		bool ssaoEnabled = false;
		bool ssrEnabled = false;
		bool irradianceVolume = false;

		for (const auto &define : curDefines)
		{
			if (std::get<0>(define) == ShaderProgram::ShaderType::FRAGMENT)
			{
				if (std::get<1>(define) == SSAO_ENABLED && std::get<2>(define))
				{
					ssaoEnabled = true;
				}
				else if (std::get<1>(define) == SSR_ENABLED && std::get<2>(define))
				{
					ssrEnabled = true;
				}
				else if (std::get<1>(define) == IRRADIANCE_VOLUME && std::get<2>(define))
				{
					irradianceVolume = true;
				}
			}
		}

		if (ssaoEnabled != (_effects.ambientOcclusion != AmbientOcclusion::OFF)
			|| ssrEnabled != _effects.screenSpaceReflections.enabled
			|| irradianceVolume != !flatAmbient)
		{
			environmentLightPassShader->setDefines(
				{
				{ ShaderProgram::ShaderType::FRAGMENT, SSAO_ENABLED, (_effects.ambientOcclusion != AmbientOcclusion::OFF) },
				{ ShaderProgram::ShaderType::FRAGMENT, SSR_ENABLED, _effects.screenSpaceReflections.enabled },
				{ ShaderProgram::ShaderType::FRAGMENT, IRRADIANCE_VOLUME, !flatAmbient },
				}
			);
			createUniforms();
		}
	}

	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, _gbuffer.ssaoTexture);
	glActiveTexture(GL_TEXTURE12);
	glBindTexture(GL_TEXTURE_2D, _level->environment.irradianceVolume->getProbeTexture()->getId());
	glActiveTexture(GL_TEXTURE13);
	glBindTexture(GL_TEXTURE_2D, _level->environment.environmentProbes[0]->getReflectanceMap()->getId());
	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, _brdfLUT);
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D, _gbuffer.lightTextures[(_renderData.frame + 1) % 2]);

	environmentLightPassShader->bind();

	uInverseViewE.set(_renderData.invViewMatrix);
	uInverseProjectionE.set(_renderData.invProjectionMatrix);

	uVolumeOrigin.set(_level->environment.irradianceVolume->getOrigin());
	uVolumeDimensions.set(_level->environment.irradianceVolume->getDimensions());
	uSpacing.set(_level->environment.irradianceVolume->getSpacing());

	static glm::mat4 prevViewProjection;

	if (!flatAmbient)
	{
		uProjectionE.set(_renderData.projectionMatrix);
		uReProjectionE.set(prevViewProjection * _renderData.invViewProjectionMatrix);
	}

	prevViewProjection = _renderData.viewProjectionMatrix;

	fullscreenTriangle->getSubMesh()->render();
}

void AmbientLightRenderPass::createUniforms()
{
	uInverseProjectionE.create(environmentLightPassShader);
	uInverseViewE.create(environmentLightPassShader);
	uProjectionE.create(environmentLightPassShader);
	uReProjectionE.create(environmentLightPassShader);

	uVolumeOrigin.create(environmentLightPassShader);
	uVolumeDimensions.create(environmentLightPassShader);
	uSpacing.create(environmentLightPassShader);
}
