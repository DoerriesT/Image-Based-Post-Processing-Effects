#include "VelocityTileMaxRenderPass.h"
#include "Graphics\OpenGL\GLTimerQuery.h"

VelocityTileMaxRenderPass::VelocityTileMaxRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
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

	m_tileMaxShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/MotionBlur/velocityTileMax.frag");

	m_uDirection.create(m_tileMaxShader);
	m_uTileSize.create(m_tileMaxShader);

	m_fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

double velocityTileMaxRenderTime;

void VelocityTileMaxRenderPass::render(GLuint _inputVelocityTexture, GLuint _intermediaryTexture, GLuint _velocityTileMaxTexture, unsigned int _tileSize, RenderPass ** _previousRenderPass)
{
	SCOPED_TIMER_QUERY(velocityTileMaxRenderTime);
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	m_fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	m_tileMaxShader->bind();

	m_uTileSize.set(_tileSize);

	glActiveTexture(GL_TEXTURE0);

	// fullscreen to first step
	{
		glViewport(0, 0, m_width / _tileSize, m_height);
		glBindTexture(GL_TEXTURE_2D, _inputVelocityTexture);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _intermediaryTexture, 0);

		m_uDirection.set(false);

		m_fullscreenTriangle->getSubMesh()->render();
	}

	// first to second step
	{
		glViewport(0, 0, m_width / _tileSize, m_height / _tileSize);
		glBindTexture(GL_TEXTURE_2D, _intermediaryTexture);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _velocityTileMaxTexture, 0);

		m_uDirection.set(true);

		m_fullscreenTriangle->getSubMesh()->render();
	}

	glViewport(0, 0, m_state.m_viewportState.m_width, m_state.m_viewportState.m_height);
}

void VelocityTileMaxRenderPass::resize(unsigned int _width, unsigned int _height)
{
	RenderPass::resize(_width, _height);
	m_width = _width;
	m_height = _height;
}
