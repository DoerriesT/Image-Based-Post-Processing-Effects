#include "FXAARenderPass.h"
#include "Graphics\Effects.h"

FXAARenderPass::FXAARenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
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

	m_fxaaShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/AntiAliasing/fxaa.frag");

	m_uInverseResolution.create(m_fxaaShader);
	m_uSubPixelAA.create(m_fxaaShader);
	m_uEdgeThreshold.create(m_fxaaShader);
	m_uEdgeThresholdMin.create(m_fxaaShader);

	m_fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void FXAARenderPass::render(const Effects & _effects, GLuint _inputTexture, GLenum _drawBuffer, RenderPass ** _previousRenderPass)
{
	if (!_effects.m_fxaa.m_enabled)
	{
		return;
	}

	m_drawBuffers[0] = _drawBuffer;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	m_fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _inputTexture);

	m_fxaaShader->bind();
	m_uSubPixelAA.set(_effects.m_fxaa.m_subPixelAA);
	m_uEdgeThreshold.set(_effects.m_fxaa.m_edgeThreshold);
	m_uEdgeThresholdMin.set(_effects.m_fxaa.m_edgeThresholdMin);
	m_uInverseResolution.set(1.0f / glm::vec2(m_state.m_viewportState.m_width, m_state.m_viewportState.m_height));

	m_fullscreenTriangle->getSubMesh()->render();
}
