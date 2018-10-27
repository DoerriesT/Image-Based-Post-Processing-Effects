#include "GTAORenderPass.h"
#include <glm\ext.hpp>
#include "Graphics\OpenGL\RenderData.h"
#include "Graphics\Effects.h"
#include "Graphics\OpenGL\GLTimerQuery.h"

GTAORenderPass::GTAORenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
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

	m_gtaoShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/SSAO/gtao.frag");

	m_uFocalLength.create(m_gtaoShader);
	m_uInverseProjection.create(m_gtaoShader);
	m_uAORes.create(m_gtaoShader);
	m_uInvAORes.create(m_gtaoShader);
	m_uStrength.create(m_gtaoShader);
	m_uRadius.create(m_gtaoShader);
	m_uMaxRadiusPixels.create(m_gtaoShader);
	m_uNumSteps.create(m_gtaoShader);
	m_uFrame.create(m_gtaoShader);

	m_fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

double gtaoRenderTime;

void GTAORenderPass::render(const RenderData &_renderData, const Effects &_effects, RenderPass **_previousRenderPass)
{
	SCOPED_TIMER_QUERY(gtaoRenderTime);
	m_drawBuffers[0] = _renderData.m_frame % 2 ? GL_COLOR_ATTACHMENT2 : GL_COLOR_ATTACHMENT0;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	m_fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	float aspectRatio = _renderData.m_resolution.second / (float)_renderData.m_resolution.first;
	float focalLength = 1.0f / tanf(glm::radians(_renderData.m_fov) * 0.5f) * aspectRatio;

	glm::vec2 res(_renderData.m_resolution.first, _renderData.m_resolution.second);

	m_gtaoShader->bind();
	m_uFrame.set(_renderData.m_frame % 12);
	m_uFocalLength.set(focalLength);
	m_uInverseProjection.set(_renderData.m_invProjectionMatrix);
	m_uAORes.set(res);
	m_uInvAORes.set(1.0f / res);
	m_uStrength.set(_effects.m_gtao.m_strength);
	m_uRadius.set(_effects.m_gtao.m_radius);
	m_uMaxRadiusPixels.set(_effects.m_gtao.m_maxRadiusPixels);
	m_uNumSteps.set((float)_effects.m_gtao.m_steps);

	m_fullscreenTriangle->getSubMesh()->render();
}
