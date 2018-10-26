#include "LensFlareBlurRenderPass.h"

LensFlareBlurRenderPass::LensFlareBlurRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	m_fbo = _fbo;
	m_drawBuffers = { GL_COLOR_ATTACHMENT2 };
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

	m_lensFlareBlurShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/LensFlares/lensFlareBlur.frag");

	m_uDirection.create(m_lensFlareBlurShader);

	m_fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void LensFlareBlurRenderPass::render(GLuint _inputTexture, GLuint _pingPongTexture, RenderPass **_previousRenderPass)
{
	m_drawBuffers[0] = GL_COLOR_ATTACHMENT2;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	m_fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	// first pass
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _inputTexture);

		m_lensFlareBlurShader->bind();
		m_uDirection.set(true);

		m_fullscreenTriangle->getSubMesh()->render();
	}
	
	// second pass
	{
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _pingPongTexture);

		m_uDirection.set(false);

		m_fullscreenTriangle->getSubMesh()->render();
	}
	

	m_drawBuffers[0] = GL_COLOR_ATTACHMENT0;
}

void LensFlareBlurRenderPass::resize(unsigned int _width, unsigned int _height)
{
	m_state.m_viewportState = { 0, 0, static_cast<GLint>(_width / 2), static_cast<GLint>(_height / 2) };
}
