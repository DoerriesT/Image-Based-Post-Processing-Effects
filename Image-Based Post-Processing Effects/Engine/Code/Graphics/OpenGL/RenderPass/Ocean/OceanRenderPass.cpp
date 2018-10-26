#include "OceanRenderPass.h"
#include "Level.h"
#include "Graphics\OpenGL\RenderData.h"
#include "Graphics\Texture.h"

OceanRenderPass::OceanRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	m_fbo = _fbo;
	m_drawBuffers = { GL_COLOR_ATTACHMENT4 };
	m_state.m_blendState.m_enabled = false;
	m_state.m_blendState.m_sFactor = GL_SRC_ALPHA;
	m_state.m_blendState.m_dFactor = GL_ONE_MINUS_SRC_ALPHA;
	m_state.m_cullFaceState.m_enabled = false;
	m_state.m_cullFaceState.m_face = GL_BACK;
	m_state.m_depthState.m_enabled = true;
	m_state.m_depthState.m_func = GL_LEQUAL;
	m_state.m_depthState.m_mask = GL_TRUE;
	m_state.m_stencilState.m_enabled = false;
	m_state.m_stencilState.m_frontFunc = m_state.m_stencilState.m_backFunc = GL_ALWAYS;
	m_state.m_stencilState.m_frontRef = m_state.m_stencilState.m_backRef = 1;
	m_state.m_stencilState.m_frontMask = m_state.m_stencilState.m_backMask = 0xFF;
	m_state.m_stencilState.m_frontOpFail = m_state.m_stencilState.m_backOpFail = GL_KEEP;
	m_state.m_stencilState.m_frontOpZfail = m_state.m_stencilState.m_backOpZfail = GL_KEEP;
	m_state.m_stencilState.m_frontOpZpass = m_state.m_stencilState.m_backOpZpass = GL_KEEP;

	resize(_width, _height);

	m_oceanShader = ShaderProgram::createShaderProgram("Resources/Shaders/Ocean/water.vert", "Resources/Shaders/Ocean/Water.frag");

	m_uProjection.create(m_oceanShader);
	m_uView.create(m_oceanShader);
	m_uCamPos.create(m_oceanShader);
	m_uTexCoordShift.create(m_oceanShader);
	m_uUseEnvironment.create(m_oceanShader);
	m_uWaterLevel.create(m_oceanShader);
	m_uLightDir.create(m_oceanShader);
	m_uLightColor.create(m_oceanShader);
}

void OceanRenderPass::render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, GLuint _displacementTexture, GLuint _normalTexture, GLuint _gridVAO, GLsizei _gridSize, bool _wireframe, RenderPass ** _previousRenderPass)
{
	resize(_renderData.m_resolution.first, _renderData.m_resolution.second);
	m_drawBuffers[0] = _renderData.m_frame % 2 ? GL_COLOR_ATTACHMENT5 : GL_COLOR_ATTACHMENT4;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	m_oceanShader->bind();

	m_uProjection.set(_renderData.m_projectionMatrix);
	m_uView.set(_renderData.m_viewMatrix);
	m_uCamPos.set(_renderData.m_cameraPosition);
	m_uTexCoordShift.set(glm::vec2(-1.5, 0.75) * _renderData.m_time * 0.25f);
	m_uUseEnvironment.set(_level->m_environment.m_environmentProbes[0]->isValid());
	m_uWaterLevel.set(_level->m_oceanParams.m_level);

	if (_level->m_lights.m_directionalLights.empty())
	{
		m_uLightDir.set(glm::normalize(glm::vec3(1.0f, 1.0f, 0.0f)));
		m_uLightColor.set(glm::vec3(1.5f, 0.575f, 0.5f));
	}
	else
	{
		m_uLightDir.set(_level->m_lights.m_directionalLights[0]->getDirection());
		m_uLightColor.set(_level->m_lights.m_directionalLights[0]->getColor());
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _normalTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _displacementTexture);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _level->m_environment.m_environmentMap->getId());

	glBindVertexArray(_gridVAO);
	glEnableVertexAttribArray(0);

	if (_wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	glDrawElements(GL_TRIANGLES, _gridSize * _gridSize * 6, GL_UNSIGNED_INT, 0);
	if (_wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}
