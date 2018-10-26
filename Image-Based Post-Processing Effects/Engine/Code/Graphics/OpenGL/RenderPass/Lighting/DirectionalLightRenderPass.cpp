#include "DirectionalLightRenderPass.h"
#include "Graphics\OpenGL\RenderData.h"
#include "Level.h"

DirectionalLightRenderPass::DirectionalLightRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
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

	directionalLightShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Lighting/directionalLight.frag");

	uDirectionalLightD.create(directionalLightShader);
	uInverseViewD.create(directionalLightShader);
	uInverseProjectionD.create(directionalLightShader);
	uShadowsEnabledD.create(directionalLightShader);

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void DirectionalLightRenderPass::render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, RenderPass **_previousRenderPass)
{
	if (_level->lights.directionalLights.empty())
	{
		return;
	}

	drawBuffers[0] = _renderData.frame % 2 ? GL_COLOR_ATTACHMENT5 : GL_COLOR_ATTACHMENT4;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	directionalLightShader->bind();

	uInverseViewD.set(_renderData.invViewMatrix);
	uInverseProjectionD.set(_renderData.invProjectionMatrix);
	uShadowsEnabledD.set(_renderData.shadows);

	for (size_t i = _level->environment.skyboxEntity ? 1 : 0; i < _level->lights.directionalLights.size(); ++i)
	{
		std::shared_ptr<DirectionalLight> directionalLight = _level->lights.directionalLights[i];
		directionalLight->updateViewValues(_renderData.viewMatrix);
		if (directionalLight->isRenderShadows())
		{
			glActiveTexture(GL_TEXTURE15);
			glBindTexture(GL_TEXTURE_2D_ARRAY, directionalLight->getShadowMap());
		}

		uDirectionalLightD.set(directionalLight);
		fullscreenTriangle->getSubMesh()->render();
	}
}
