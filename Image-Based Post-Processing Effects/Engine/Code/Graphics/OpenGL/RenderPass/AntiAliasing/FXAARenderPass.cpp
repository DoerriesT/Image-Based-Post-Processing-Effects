#include "FXAARenderPass.h"
#include "Graphics\Effects.h"

FXAARenderPass::FXAARenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
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

	fxaaShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/AntiAliasing/fxaa.frag");

	uInverseResolutionF.create(fxaaShader);
	uSubPixelAAF.create(fxaaShader);
	uEdgeThresholdF.create(fxaaShader);
	uEdgeThresholdMinF.create(fxaaShader);

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void FXAARenderPass::render(const Effects & _effects, GLuint _inputTexture, GLenum _drawBuffer, RenderPass ** _previousRenderPass)
{
	if (!_effects.fxaa.enabled)
	{
		return;
	}

	drawBuffers[0] = _drawBuffer;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _inputTexture);

	fxaaShader->bind();
	uSubPixelAAF.set(_effects.fxaa.subPixelAA);
	uEdgeThresholdF.set(_effects.fxaa.edgeThreshold);
	uEdgeThresholdMinF.set(_effects.fxaa.edgeThresholdMin);
	uInverseResolutionF.set(1.0f / glm::vec2(state.viewportState.width, state.viewportState.height));

	fullscreenTriangle->getSubMesh()->render();
}
