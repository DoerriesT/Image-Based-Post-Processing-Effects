#include "SMAAEdgeDetectionRenderPass.h"

SMAAEdgeDetectionRenderPass::SMAAEdgeDetectionRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	m_fbo = _fbo;
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

	resize(_width, _height);

	m_edgeDetectionShader = ShaderProgram::createShaderProgram("Resources/Shaders/AntiAliasing/smaaEdgeDetection.vert", "Resources/Shaders/AntiAliasing/smaaEdgeDetection.frag");

	m_uResolution.create(m_edgeDetectionShader);

	m_fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void SMAAEdgeDetectionRenderPass::render(const Effects & _effects, GLuint _inputTexture, RenderPass **_previousRenderPass)
{
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	m_fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	glClear(GL_COLOR_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _inputTexture);

	m_edgeDetectionShader->bind();

	m_uResolution.set(glm::vec4(1.0f / m_state.m_viewportState.m_width, 1.0f / m_state.m_viewportState.m_height, m_state.m_viewportState.m_width, m_state.m_viewportState.m_height));

	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	m_fullscreenTriangle->getSubMesh()->render();
}
