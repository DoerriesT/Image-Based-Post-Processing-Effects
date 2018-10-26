#include "LensFlareBlurRenderPass.h"

LensFlareBlurRenderPass::LensFlareBlurRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	fbo = _fbo;
	drawBuffers = { GL_COLOR_ATTACHMENT2 };
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

	lensFlareBlurShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/LensFlares/lensFlareBlur.frag");

	uDirectionLFB.create(lensFlareBlurShader);

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void LensFlareBlurRenderPass::render(GLuint _inputTexture, GLuint _pingPongTexture, RenderPass **_previousRenderPass)
{
	drawBuffers[0] = GL_COLOR_ATTACHMENT2;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	// first pass
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _inputTexture);

		lensFlareBlurShader->bind();
		uDirectionLFB.set(true);

		fullscreenTriangle->getSubMesh()->render();
	}
	
	// second pass
	{
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _pingPongTexture);

		uDirectionLFB.set(false);

		fullscreenTriangle->getSubMesh()->render();
	}
	

	drawBuffers[0] = GL_COLOR_ATTACHMENT0;
}

void LensFlareBlurRenderPass::resize(unsigned int _width, unsigned int _height)
{
	state.viewportState = { 0, 0, static_cast<GLint>(_width / 2), static_cast<GLint>(_height / 2) };
}
