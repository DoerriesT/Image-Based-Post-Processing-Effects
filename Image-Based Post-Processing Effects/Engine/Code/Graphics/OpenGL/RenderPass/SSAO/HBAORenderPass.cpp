#include "HBAORenderPass.h"
#include <glm\ext.hpp>
#include "Graphics\OpenGL\RenderData.h"
#include "Graphics\Effects.h"
#include "Graphics\OpenGL\GLTimerQuery.h"

HBAORenderPass::HBAORenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
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

	m_hbaoShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/SSAO/hbao.frag");

	m_uFocalLength.create(m_hbaoShader);
	m_uInverseProjection.create(m_hbaoShader);
	m_uAORes.create(m_hbaoShader);
	m_uInvAORes.create(m_hbaoShader);
	m_uNoiseScale.create(m_hbaoShader);
	m_uStrength.create(m_hbaoShader);
	m_uRadius.create(m_hbaoShader);
	m_uRadius2.create(m_hbaoShader);
	m_uNegInvR2.create(m_hbaoShader);
	m_uTanBias.create(m_hbaoShader);
	m_uMaxRadiusPixels.create(m_hbaoShader);
	m_uNumDirections.create(m_hbaoShader);
	m_uNumSteps.create(m_hbaoShader);

	m_fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

double hbaoRenderTime;

void HBAORenderPass::render(const RenderData & _renderData, const Effects & _effects, GLuint _noiseTexture, RenderPass **_previousRenderPass)
{
	GLTimerQuery timer(hbaoRenderTime);
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	m_fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, _noiseTexture);

	float aspectRatio = _renderData.m_resolution.second / (float)_renderData.m_resolution.first;
	glm::vec2 focalLength;
	focalLength.x = 1.0f / tanf(glm::radians(_renderData.m_fov) * 0.5f) * aspectRatio;
	focalLength.y = 1.0f / tanf(glm::radians(_renderData.m_fov) * 0.5f);

	glm::vec2 res(_renderData.m_resolution.first, _renderData.m_resolution.second);

	m_hbaoShader->bind();
	m_uFocalLength.set(focalLength);
	m_uInverseProjection.set(_renderData.m_invProjectionMatrix);
	m_uAORes.set(res);
	m_uInvAORes.set(1.0f / res);
	m_uNoiseScale.set(res * 0.25f);
	m_uStrength.set(_effects.m_hbao.m_strength);
	m_uRadius.set(_effects.m_hbao.m_radius);
	m_uRadius2.set(_effects.m_hbao.m_radius * _effects.m_hbao.m_radius);
	m_uNegInvR2.set(-1.0f / (_effects.m_hbao.m_radius * _effects.m_hbao.m_radius));
	m_uTanBias.set(_effects.m_hbao.m_angleBias);
	m_uMaxRadiusPixels.set(_effects.m_hbao.m_maxRadiusPixels);
	m_uNumDirections.set((float)_effects.m_hbao.m_directions);
	m_uNumSteps.set((float)_effects.m_hbao.m_steps);

	m_fullscreenTriangle->getSubMesh()->render();
}
