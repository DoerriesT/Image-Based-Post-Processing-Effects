#include "SSAOBilateralBlurRenderPass.h"
#include "Graphics\OpenGL\RenderData.h"
#include "Graphics\Effects.h"

SSAOBilateralBlurRenderPass::SSAOBilateralBlurRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	fbo = _fbo;
	drawBuffers = { GL_COLOR_ATTACHMENT1 };
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

	ssaoBilateralBlurShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/SSAO/ssaoBilateralBlur.frag");

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void SSAOBilateralBlurRenderPass::render(const RenderData & _renderData, const Effects & _effects, GLuint _ssaoTexture, RenderPass **_previousRenderPass)
{
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	ssaoBilateralBlurShader->bind();

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, _ssaoTexture);

	fullscreenTriangle->getSubMesh()->render();
}
