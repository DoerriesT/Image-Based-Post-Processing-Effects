#include "GTAOTemporalDenoiseRenderPass.h"
#include "Engine.h"
#include "Graphics\OpenGL\RenderData.h"
#include "Graphics\OpenGL\GLTimerQuery.h"

GTAOTemporalDenoiseRenderPass::GTAOTemporalDenoiseRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
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

	m_gtaoDenoiseShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/SSAO/gtaoTemporalDenoise.frag");

	m_uFrameTime.create(m_gtaoDenoiseShader);
	m_uInvProjection.create(m_gtaoDenoiseShader);
	m_uInvView.create(m_gtaoDenoiseShader);
	m_uPrevInvProjection.create(m_gtaoDenoiseShader);
	m_uPrevInvView.create(m_gtaoDenoiseShader);

	m_fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

double gtaoTemporalDenoiseTime;

void GTAOTemporalDenoiseRenderPass::render(const RenderData & _renderData, const Effects & _effects, GLuint _velocityTexture, GLuint * _ssaoTextures, RenderPass **_previousRenderPass)
{
	SCOPED_TIMER_QUERY(gtaoTemporalDenoiseTime);
	m_drawBuffers[0] = _renderData.m_frame % 2 ? GL_COLOR_ATTACHMENT2 : GL_COLOR_ATTACHMENT0;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	m_gtaoDenoiseShader->bind();

	glm::vec2 texelSize = 1.0f / glm::vec2(_renderData.m_resolution.first, _renderData.m_resolution.second);
	m_uFrameTime.set((float)Engine::getTimeDelta());
	m_uInvProjection.set(_renderData.m_invProjectionMatrix);
	m_uInvView.set(_renderData.m_invViewMatrix);
	m_uPrevInvProjection.set(_renderData.m_prevInvProjectionMatrix);
	m_uPrevInvView.set(_renderData.m_prevInvViewMatrix);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, _velocityTexture);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, _renderData.m_frame % 2 ? _ssaoTextures[0] : _ssaoTextures[2]);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, _ssaoTextures[1]);

	m_fullscreenTriangle->getSubMesh()->render();
}
