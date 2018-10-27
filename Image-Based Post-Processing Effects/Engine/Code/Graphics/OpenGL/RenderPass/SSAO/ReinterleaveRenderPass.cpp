#include "ReinterleaveRenderPass.h"

ReinterleaveRenderPass::ReinterleaveRenderPass(GLuint fbo, unsigned int width, unsigned int height)
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

	m_reinterleaveShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/SSAO/reinterleave.frag");

	m_fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void ReinterleaveRenderPass::render(GLuint sourceArrayTexture, GLuint targetTexture, RenderPass ** previousRenderPass)
{
	RenderPass::begin(*previousRenderPass);
	*previousRenderPass = this;

	m_reinterleaveShader->bind();

	m_fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, sourceArrayTexture);

	m_fullscreenTriangle->getSubMesh()->render();
}
