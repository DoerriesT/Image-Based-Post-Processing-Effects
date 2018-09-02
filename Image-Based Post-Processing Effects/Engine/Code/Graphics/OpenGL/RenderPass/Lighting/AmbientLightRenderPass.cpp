#include "AmbientLightRenderPass.h"
#include "Graphics\OpenGL\RenderData.h"
#include "Level.h"
#include "Graphics\Effects.h"
#include "Graphics\Texture.h"

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

	environmentLightPassShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Renderer/globalLightPass.frag");

	uInverseProjectionE.create(environmentLightPassShader);
	uInverseViewE.create(environmentLightPassShader);
	uDirectionalLightE.create(environmentLightPassShader);
	uShadowsEnabledE.create(environmentLightPassShader);
	uRenderDirectionalLightE.create(environmentLightPassShader);
	uSsaoE.create(environmentLightPassShader);
	uProjectionE.create(environmentLightPassShader);
	uReProjectionE.create(environmentLightPassShader);
	uUseSsrE.create(environmentLightPassShader);

	uVolumeOrigin.create(environmentLightPassShader);
	uVolumeDimensions.create(environmentLightPassShader);
	uSpacing.create(environmentLightPassShader);

	uFlatAmbient.create(environmentLightPassShader);

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

bool flatAmbient;

void AmbientLightRenderPass::render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const Effects &_effects, const GBuffer &_gbuffer, GLuint _brdfLUT, RenderPass **_previousRenderPass)
{
	drawBuffers[0] = _renderData.frame % 2 ? GL_COLOR_ATTACHMENT5 : GL_COLOR_ATTACHMENT4;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

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
	uProjectionE.set(_renderData.projectionMatrix);
	uInverseProjectionE.set(_renderData.invProjectionMatrix);
	uSsaoE.set(_effects.ambientOcclusion != AmbientOcclusion::OFF);
	uUseSsrE.set(_effects.screenSpaceReflections.enabled);

	uVolumeOrigin.set(_level->environment.irradianceVolume->getOrigin());
	uVolumeDimensions.set(_level->environment.irradianceVolume->getDimensions());
	uSpacing.set(_level->environment.irradianceVolume->getSpacing());

	uFlatAmbient.set(flatAmbient);

	static glm::mat4 prevViewProjection;

	uReProjectionE.set(prevViewProjection * _renderData.invViewProjectionMatrix);
	prevViewProjection = _renderData.viewProjectionMatrix;

	if (!_level->lights.directionalLights.empty())
	{
		std::shared_ptr<DirectionalLight> directionalLight = _level->lights.directionalLights[0];
		directionalLight->updateViewValues(_renderData.viewMatrix);
		if (directionalLight->isRenderShadows())
		{
			glActiveTexture(GL_TEXTURE15);
			glBindTexture(GL_TEXTURE_2D_ARRAY, directionalLight->getShadowMap());
		}
		uDirectionalLightE.set(directionalLight);
		uRenderDirectionalLightE.set(true);
		uShadowsEnabledE.set(_renderData.shadows);
	}
	else
	{
		uRenderDirectionalLightE.set(false);
	}

	fullscreenTriangle->getSubMesh()->render();
}
