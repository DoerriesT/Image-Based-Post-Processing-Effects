#include "SMAABlendRenderPass.h"

SMAABlendRenderPass::SMAABlendRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
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

	m_blendShader = ShaderProgram::createShaderProgram("Resources/Shaders/AntiAliasing/smaaNeighborhoodBlending.vert", "Resources/Shaders/AntiAliasing/smaaNeighborhoodBlending.frag");

	m_uResolution.create(m_blendShader);

	m_fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void SMAABlendRenderPass::render(const Effects & _effects, GLuint _inputTexture, GLuint _velocityTexture, GLuint _blendWeightTexture, bool _currentSample, RenderPass ** _previousRenderPass)
{
	m_drawBuffers[0] = GL_COLOR_ATTACHMENT2 + static_cast<int>(_currentSample);
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	m_fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _inputTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _blendWeightTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, _velocityTexture);

	m_blendShader->bind();

	m_uResolution.set(glm::vec4(1.0f / m_state.m_viewportState.m_width, 1.0f / m_state.m_viewportState.m_height, m_state.m_viewportState.m_width, m_state.m_viewportState.m_height));

	m_fullscreenTriangle->getSubMesh()->render();
}
