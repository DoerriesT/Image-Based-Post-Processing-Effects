#include "LightInjectionRenderPass.h"

LightInjectionRenderPass::LightInjectionRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	fbo = _fbo;
	drawBuffers = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
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

	lightInjectionShader = ShaderProgram::createShaderProgram("Resources/Shaders/Renderer/lightInjection.vert", "Resources/Shaders/Renderer/lightInjection.frag");

	uInvViewProjection.create(lightInjectionShader);
	uRsmWidth.create(lightInjectionShader);
	uGridOrigin.create(lightInjectionShader);
	uGridSize.create(lightInjectionShader);
	uGridSpacing.create(lightInjectionShader);

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void LightInjectionRenderPass::render(const LightPropagationVolume &_lightPropagationVolume, const glm::mat4 &_invViewProjection, GLint _depthTexture, GLint _fluxTexture, GLint _normalTexture, RenderPass **_previousRenderPass)
{
}
