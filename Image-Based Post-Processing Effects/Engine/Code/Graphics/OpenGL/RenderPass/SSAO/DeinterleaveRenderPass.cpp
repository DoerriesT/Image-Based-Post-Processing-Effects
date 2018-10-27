#include "DeinterleaveRenderPass.h"

DeinterleaveRenderPass::DeinterleaveRenderPass(GLuint fbo, unsigned int width, unsigned int height)
{
	m_fbo = fbo;
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

	resize(width, height);

	m_deinterleaveShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/SSAO/deinterleave.frag");

	m_uOffset.create(m_deinterleaveShader);

	m_fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void DeinterleaveRenderPass::render(GLuint sourceTexture, GLuint *targetTextures, RenderPass **previousRenderPass)
{
	RenderPass::begin(*previousRenderPass);
	*previousRenderPass = this;

	m_deinterleaveShader->bind();

	m_fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sourceTexture);

	for (unsigned int i = 0; i < 2; ++i)
	{
		for (unsigned int j = 0; j < 8; ++j)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + j, GL_TEXTURE_2D, targetTextures[j], 0);
		}

		m_uOffset.set(glm::vec2(0.5f, i * 2 + 0.5f));

		m_fullscreenTriangle->getSubMesh()->render();
	}
}

void DeinterleaveRenderPass::resize(unsigned int width, unsigned int height)
{
	width = (width + 3) / 4;
	height = (height + 3) / 4;
	m_state.m_viewportState = { 0, 0, static_cast<GLint>(width), static_cast<GLint>(height) };
}
