#include "OceanTesselationRenderPass.h"
#include "Level.h"
#include "Graphics\OpenGL\RenderData.h"
#include "Graphics\Texture.h"
#include "Graphics\OpenGL\TileRing.h"
#include "Engine.h"

OceanTesselationRenderPass::OceanTesselationRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
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

	m_oceanTesselationShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/terrain.vert", "Resources/Shaders/Ocean/water.frag", "Resources/Shaders/Shared/terrain.tessc", "Resources/Shaders/Shared/terrain.tesse");

	m_uViewProjection.create(m_oceanTesselationShader);
	m_uProjection.create(m_oceanTesselationShader);
	m_uView.create(m_oceanTesselationShader);
	m_uCamPos.create(m_oceanTesselationShader);
	m_uTexCoordShift.create(m_oceanTesselationShader);
	m_uUseEnvironment.create(m_oceanTesselationShader);
	m_uWaterLevel.create(m_oceanTesselationShader);
	m_uLightDir.create(m_oceanTesselationShader);
	m_uLightColor.create(m_oceanTesselationShader);
	m_uTileSize.create(m_oceanTesselationShader);
	m_uViewDir.create(m_oceanTesselationShader);
	m_uScreenSize.create(m_oceanTesselationShader);
	m_uTesselatedTriWidth.create(m_oceanTesselationShader);
	m_uTexCoordScale.create(m_oceanTesselationShader);
	m_uDisplacementScale.create(m_oceanTesselationShader);
	m_uPerlinMovement.create(m_oceanTesselationShader);

	m_perlinNoiseTexture = Texture::createTexture("Resources/Textures/perlin_noise.dds", true);
}

void OceanTesselationRenderPass::render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, GLuint _displacementTexture, GLuint _normalTexture, TileRing **_tileRings, bool _wireframe, RenderPass ** _previousRenderPass)
{
	resize(_renderData.m_resolution.first, _renderData.m_resolution.second);
	m_drawBuffers[0] = _renderData.m_frame % 2 ? GL_COLOR_ATTACHMENT5 : GL_COLOR_ATTACHMENT4;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	m_oceanTesselationShader->bind();
	m_uViewProjection.set(_renderData.m_viewProjectionMatrix);
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

	m_uViewDir.set(_renderData.m_viewDirection);
	m_uScreenSize.set(glm::vec2(_renderData.m_resolution.first, _renderData.m_resolution.second));
	m_uTesselatedTriWidth.set(20);
	m_uTexCoordScale.set(1.0f / 20.0f);
	m_uDisplacementScale.set(1.0f);

	m_uPerlinMovement.set(_level->m_oceanParams.m_normalizedWindDirection * float(Engine::getTime()) * -0.01f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _normalTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _displacementTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m_perlinNoiseTexture->getId());
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _level->m_environment.m_environmentMap->getId());

	m_uTileSize.set(1.0f);

	if (_wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	for (int i = 0; i < 6; ++i)
	{
		m_uTileSize.set(_tileRings[i]->getTileSize());
		_tileRings[i]->render();
	}
	if (_wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}
