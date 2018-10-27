#include "SSAOBilateralBlurRenderPass.h"
#include "Graphics\OpenGL\RenderData.h"
#include "Graphics\Effects.h"
#include "Graphics\OpenGL\GLTimerQuery.h"

SSAOBilateralBlurRenderPass::SSAOBilateralBlurRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	m_fbo = _fbo;
	m_drawBuffers = { GL_COLOR_ATTACHMENT1 };
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

	m_ssaoBilateralBlurShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/SSAO/ssaoBilateralBlur.frag");

	m_fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

double bilateralBlurRenderTime;

void SSAOBilateralBlurRenderPass::render(const RenderData & _renderData, const Effects & _effects, GLuint _ssaoTexture, RenderPass **_previousRenderPass)
{
	GLTimerQuery timer(bilateralBlurRenderTime);
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	m_ssaoBilateralBlurShader->bind();

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, _ssaoTexture);

	m_fullscreenTriangle->getSubMesh()->render();
}
