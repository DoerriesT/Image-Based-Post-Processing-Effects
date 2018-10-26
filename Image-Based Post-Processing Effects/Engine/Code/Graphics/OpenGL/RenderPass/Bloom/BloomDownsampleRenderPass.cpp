#include "BloomDownsampleRenderPass.h"

BloomDownsampleRenderPass::BloomDownsampleRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
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

	m_downsampleShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Misc/downsample.frag");

	m_fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void BloomDownsampleRenderPass::render(GLuint * _textureChain, size_t _textureCount, GLuint *_fbos, size_t _fboCount, RenderPass ** _previousRenderPass)
{
	m_fbo = _fbos[0];
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	glm::vec2 viewports[] =
	{
		glm::vec2(m_state.m_viewportState.m_width, m_state.m_viewportState.m_height),
		glm::vec2(m_state.m_viewportState.m_width / 2.0f, m_state.m_viewportState.m_height / 2.0f),
		glm::vec2(m_state.m_viewportState.m_width / 4.0f, m_state.m_viewportState.m_height / 4.0f),
		glm::vec2(m_state.m_viewportState.m_width / 8.0f, m_state.m_viewportState.m_height / 8.0f),
		glm::vec2(m_state.m_viewportState.m_width / 16.0f, m_state.m_viewportState.m_height / 16.0f),
		glm::vec2(m_state.m_viewportState.m_width / 32.0f, m_state.m_viewportState.m_height / 32.0f)
	};

	m_downsampleShader->bind();

	for (size_t i = 0; i < _fboCount; ++i)
	{
		if (i > 0)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, _fbos[i]);
			glViewport(0, 0, (GLsizei)viewports[i].x, (GLsizei)viewports[i].y);
		}
		

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _textureChain[i]);

		m_fullscreenTriangle->getSubMesh()->render();
	}

	m_fbo = _fbos[_fboCount - 1];
	glViewport(0, 0, (GLsizei)viewports[0].x, (GLsizei)viewports[0].y);
}
