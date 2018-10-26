#include "DirectionalLightRenderPass.h"
#include "Graphics\OpenGL\RenderData.h"
#include "Level.h"

DirectionalLightRenderPass::DirectionalLightRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	m_fbo = _fbo;
	m_drawBuffers = { GL_COLOR_ATTACHMENT4 };
	m_state.m_blendState.m_enabled = true;
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

	m_directionalLightShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Lighting/directionalLight.frag");

	m_uDirectionalLight.create(m_directionalLightShader);
	m_uInverseView.create(m_directionalLightShader);
	m_uInverseProjection.create(m_directionalLightShader);
	m_uShadowsEnabled.create(m_directionalLightShader);

	m_fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void DirectionalLightRenderPass::render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, RenderPass **_previousRenderPass)
{
	if (_level->m_lights.m_directionalLights.empty())
	{
		return;
	}

	m_drawBuffers[0] = _renderData.m_frame % 2 ? GL_COLOR_ATTACHMENT5 : GL_COLOR_ATTACHMENT4;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	m_fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	m_directionalLightShader->bind();

	m_uInverseView.set(_renderData.m_invViewMatrix);
	m_uInverseProjection.set(_renderData.m_invProjectionMatrix);
	m_uShadowsEnabled.set(_renderData.m_shadows);

	for (size_t i = _level->m_environment.m_skyboxEntity ? 1 : 0; i < _level->m_lights.m_directionalLights.size(); ++i)
	{
		std::shared_ptr<DirectionalLight> directionalLight = _level->m_lights.m_directionalLights[i];
		directionalLight->updateViewValues(_renderData.m_viewMatrix);
		if (directionalLight->isRenderShadows())
		{
			glActiveTexture(GL_TEXTURE15);
			glBindTexture(GL_TEXTURE_2D_ARRAY, directionalLight->getShadowMap());
		}

		m_uDirectionalLight.set(directionalLight);
		m_fullscreenTriangle->getSubMesh()->render();
	}
}
