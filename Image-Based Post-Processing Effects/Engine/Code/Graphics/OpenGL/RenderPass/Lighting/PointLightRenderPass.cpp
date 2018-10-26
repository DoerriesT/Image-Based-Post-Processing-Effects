#include "PointLightRenderPass.h"
#include <glm\mat4x4.hpp>
#include <glm\vec3.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\ext.hpp>
#include "Level.h"
#include "Graphics\OpenGL\RenderData.h"

PointLightRenderPass::PointLightRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
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

	m_pointLightPassShader = ShaderProgram::createShaderProgram("Resources/Shaders/Lighting/lightProxy.vert", "Resources/Shaders/Lighting/pointLight.frag");

	m_uModelViewProjection.create(m_pointLightPassShader);
	m_uPointLight.create(m_pointLightPassShader);
	m_uInverseProjection.create(m_pointLightPassShader);
	m_uInverseView.create(m_pointLightPassShader);
	m_uShadowsEnabled.create(m_pointLightPassShader);
	m_uViewportSize.create(m_pointLightPassShader);

	m_pointLightMesh = Mesh::createMesh("Resources/Models/pointlight.mesh", 1, true);
}

void PointLightRenderPass::render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, RenderPass **_previousRenderPass)
{
	if (_level->m_lights.m_pointLights.empty())
	{
		return;
	}

	m_drawBuffers[0] = _renderData.m_frame % 2 ? GL_COLOR_ATTACHMENT5 : GL_COLOR_ATTACHMENT4;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	m_pointLightMesh->getSubMesh()->enableVertexAttribArrays();

	m_pointLightPassShader->bind();

	m_uInverseView.set(_renderData.m_invViewMatrix);
	m_uInverseProjection.set(_renderData.m_invProjectionMatrix);
	m_uShadowsEnabled.set(_renderData.m_shadows);
	m_uViewportSize.set(glm::vec2(_renderData.m_resolution.first, _renderData.m_resolution.second));

	for (std::shared_ptr<PointLight> pointLight : _level->m_lights.m_pointLights)
	{
		if (_renderData.m_bake && pointLight->getMobility() != Mobility::STATIC)
		{
			continue;
		}

		if (!_renderData.m_frustum.testSphere(pointLight->getBoundingSphere()))
		{
			continue;
		}

		pointLight->updateViewValues(_renderData.m_viewMatrix);

		if (pointLight->isRenderShadows())
		{
			glActiveTexture(GL_TEXTURE14);
			glBindTexture(GL_TEXTURE_CUBE_MAP, pointLight->getShadowMap());
		}

		m_uModelViewProjection.set(_renderData.m_viewProjectionMatrix * glm::translate(pointLight->getPosition()) * glm::scale(glm::vec3(pointLight->getRadius() + 0.1f)));
		m_uPointLight.set(pointLight);
		m_pointLightMesh->getSubMesh()->render();
	}
}
