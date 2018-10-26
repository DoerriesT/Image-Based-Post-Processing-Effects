#include "DeferredEnvironmentProbeRenderPass.h"
#include <glm\mat4x4.hpp>
#include <glm\vec3.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\ext.hpp>
#include "Level.h"
#include "Graphics\OpenGL\RenderData.h"
#include "Graphics\Texture.h"
#include "Graphics\Effects.h"

static const char *SSAO_ENABLED = "SSAO_ENABLED";

DeferredEnvironmentProbeRenderPass::DeferredEnvironmentProbeRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	m_fbo = _fbo;
	m_drawBuffers = { GL_COLOR_ATTACHMENT4 };
	m_state.m_blendState.m_enabled = true;
	m_state.m_blendState.m_sFactor = GL_ONE;
	m_state.m_blendState.m_dFactor = GL_ONE;
	m_state.m_cullFaceState.m_enabled = true;
	m_state.m_cullFaceState.m_face = GL_FRONT;
	m_state.m_depthState.m_enabled = false;
	m_state.m_depthState.m_func = GL_LEQUAL;
	m_state.m_depthState.m_mask = GL_FALSE;
	m_state.m_stencilState.m_enabled = true;
	m_state.m_stencilState.m_frontFunc = m_state.m_stencilState.m_backFunc = GL_NOTEQUAL;
	m_state.m_stencilState.m_frontRef = m_state.m_stencilState.m_backRef = 0;
	m_state.m_stencilState.m_frontMask = m_state.m_stencilState.m_backMask = 0xFF;
	m_state.m_stencilState.m_frontOpFail = m_state.m_stencilState.m_backOpFail = GL_KEEP;
	m_state.m_stencilState.m_frontOpZfail = m_state.m_stencilState.m_backOpZfail = GL_KEEP;
	m_state.m_stencilState.m_frontOpZpass = m_state.m_stencilState.m_backOpZpass = GL_KEEP;

	resize(_width, _height);

	m_deferredEnvironmentProbePassShader = ShaderProgram::createShaderProgram(
		{
		{ ShaderProgram::ShaderType::FRAGMENT, SSAO_ENABLED, 0 },
		},
		"Resources/Shaders/Lighting/deferredEnvironmentProbe.vert", 
		"Resources/Shaders/Lighting/deferredEnvironmentProbe.frag");

	m_uModelViewProjection.create(m_deferredEnvironmentProbePassShader);
	m_uInverseView.create(m_deferredEnvironmentProbePassShader);
	m_uInverseProjection.create(m_deferredEnvironmentProbePassShader);
	m_uBoxMin.create(m_deferredEnvironmentProbePassShader);
	m_uBoxMax.create(m_deferredEnvironmentProbePassShader);
	m_uProbePosition.create(m_deferredEnvironmentProbePassShader);

	m_boxMesh = Mesh::createMesh("Resources/Models/cube.mesh", 1, true);
}

void DeferredEnvironmentProbeRenderPass::render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const Effects &_effects, GLuint _ssaoTexture, GLuint _brdfLUT, RenderPass **_previousRenderPass)
{
	if (_level->m_environment.m_environmentProbes.empty())
	{
		return;
	}

	m_drawBuffers[0] = _renderData.m_frame % 2 ? GL_COLOR_ATTACHMENT5 : GL_COLOR_ATTACHMENT4;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	// shader permutations
	{
		const auto curDefines = m_deferredEnvironmentProbePassShader->getDefines();

		bool ssaoEnabled = false;

		for (const auto &define : curDefines)
		{
			if (std::get<0>(define) == ShaderProgram::ShaderType::FRAGMENT)
			{
				if (std::get<1>(define) == SSAO_ENABLED && std::get<2>(define))
				{
					ssaoEnabled = true;
				}
			}
		}

		if (ssaoEnabled != (_effects.m_ambientOcclusion != AmbientOcclusion::OFF))
		{
			m_deferredEnvironmentProbePassShader->setDefines(
				{
				{ ShaderProgram::ShaderType::FRAGMENT, SSAO_ENABLED, (_effects.m_ambientOcclusion != AmbientOcclusion::OFF) },
				}
			);
			m_uModelViewProjection.create(m_deferredEnvironmentProbePassShader);
			m_uInverseView.create(m_deferredEnvironmentProbePassShader);
			m_uInverseProjection.create(m_deferredEnvironmentProbePassShader);
			m_uBoxMin.create(m_deferredEnvironmentProbePassShader);
			m_uBoxMax.create(m_deferredEnvironmentProbePassShader);
			m_uProbePosition.create(m_deferredEnvironmentProbePassShader);
		}
	}

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, _ssaoTexture);
	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, _brdfLUT);

	m_boxMesh->getSubMesh()->enableVertexAttribArraysPositionOnly();

	m_deferredEnvironmentProbePassShader->bind();

	m_uInverseView.set(_renderData.m_invViewMatrix);
	m_uInverseProjection.set(_renderData.m_invProjectionMatrix);

	for (auto probe : _level->m_environment.m_environmentProbes)
	{
		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_2D, probe->getReflectionTexture()->getId());

		AxisAlignedBoundingBox aabb = probe->getAxisAlignedBoundingBox();
		glm::vec3 boundingBoxCenter = (aabb.m_max + aabb.m_min) * 0.5f;
		glm::vec3 correctedMax = aabb.m_max - boundingBoxCenter;
		glm::vec3 correctedMin = aabb.m_min - boundingBoxCenter;
		glm::vec3 boxScale = correctedMax / 0.5f;

		glm::mat4 modelMatrix = glm::translate(boundingBoxCenter)
			* glm::scale(glm::vec3(boxScale));

		m_uBoxMin.set(aabb.m_min);
		m_uBoxMax.set(aabb.m_max);
		m_uProbePosition.set(probe->getPosition());
		m_uModelViewProjection.set(_renderData.m_viewProjectionMatrix * modelMatrix);

		m_boxMesh->getSubMesh()->render();
	}
}
