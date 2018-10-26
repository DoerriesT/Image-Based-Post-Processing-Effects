#include "SimplePostEffectsRenderPass.h"
#include "Engine.h"
#include "Graphics\Effects.h"

SimplePostEffectsRenderPass::SimplePostEffectsRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
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

	m_simplePostEffectsShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Misc/singlePassEffects.frag");

	m_uTime.create(m_simplePostEffectsShader);
	m_uFilmGrainStrength.create(m_simplePostEffectsShader);
	m_uVignette.create(m_simplePostEffectsShader);
	m_uFilmGrain.create(m_simplePostEffectsShader);
	m_uChromaticAberration.create(m_simplePostEffectsShader);
	m_uChromAbOffsetMultiplier.create(m_simplePostEffectsShader);

	m_fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void SimplePostEffectsRenderPass::render(const Effects & _effects, GLuint _inputTexture, GLenum _drawBuffer, RenderPass ** _previousRenderPass)
{
	m_drawBuffers[0] = _drawBuffer;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	m_fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _inputTexture);

	m_simplePostEffectsShader->bind();
	m_uTime.set((float)Engine::getTime());
	m_uFilmGrainStrength.set(_effects.m_filmGrain.m_strength);
	m_uVignette.set(_effects.m_vignette.m_enabled);
	m_uFilmGrain.set(_effects.m_filmGrain.m_enabled);
	m_uChromaticAberration.set(_effects.m_chromaticAberration.m_enabled);
	m_uChromAbOffsetMultiplier.set(_effects.m_chromaticAberration.m_offsetMultiplier);

	m_fullscreenTriangle->getSubMesh()->render();
}
