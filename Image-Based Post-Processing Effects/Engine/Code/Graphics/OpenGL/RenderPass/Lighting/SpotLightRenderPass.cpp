#include "SpotLightRenderPass.h"
#include <glm\mat4x4.hpp>
#include <glm\vec3.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\ext.hpp>
#include "Level.h"
#include "Graphics\OpenGL\RenderData.h"
#include "Graphics\Texture.h"

SpotLightRenderPass::SpotLightRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
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

	m_spotLightPassShader = ShaderProgram::createShaderProgram("Resources/Shaders/Lighting/lightProxy.vert", "Resources/Shaders/Lighting/spotLight.frag");

	m_uModelViewProjection.create(m_spotLightPassShader);
	m_uSpotLight.create(m_spotLightPassShader);
	m_uInverseView.create(m_spotLightPassShader);
	m_uInverseProjection.create(m_spotLightPassShader);
	m_uShadowsEnabled.create(m_spotLightPassShader);
	m_uViewportSize.create(m_spotLightPassShader);

	m_spotLightMesh = Mesh::createMesh("Resources/Models/spotlight.mesh", 1, true);
}

void SpotLightRenderPass::render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, RenderPass **_previousRenderPass)
{
	if (_level->m_lights.m_spotLights.empty())
	{
		return;
	}

	m_drawBuffers[0] = _renderData.m_frame % 2 ? GL_COLOR_ATTACHMENT5 : GL_COLOR_ATTACHMENT4;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	m_spotLightMesh->getSubMesh()->enableVertexAttribArrays();

	m_spotLightPassShader->bind();

	m_uInverseView.set(_renderData.m_invViewMatrix);
	m_uInverseProjection.set(_renderData.m_invProjectionMatrix);
	m_uShadowsEnabled.set(_renderData.m_shadows);
	m_uViewportSize.set(glm::vec2(_renderData.m_resolution.first, _renderData.m_resolution.second));

	for (std::shared_ptr<SpotLight> spotLight : _level->m_lights.m_spotLights)
	{
		if (_renderData.m_bake && spotLight->getMobility() != Mobility::STATIC)
		{
			continue;
		}

		if (!_renderData.m_frustum.testSphere(spotLight->getBoundingSphere()))
		{
			continue;
		}

		spotLight->updateViewValues(_renderData.m_viewMatrix);

		if (spotLight->isRenderShadows())
		{
			glActiveTexture(GL_TEXTURE14);
			glBindTexture(GL_TEXTURE_2D, spotLight->getShadowMap());
		}
		if (spotLight->isProjector())
		{
			assert(spotLight->getProjectionTexture());
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, spotLight->getProjectionTexture()->getId());
		}

		// scale a bit larger to correct for proxy geometry not being exactly round
		float scale = (glm::tan(spotLight->getOuterAngle() * 0.5f) + 0.1f) * spotLight->getRadius();

		const glm::vec3 defaultDirection = glm::vec3(0.0f, -1.0f, 0.0f);

		m_uModelViewProjection.set(_renderData.m_viewProjectionMatrix
			* glm::translate(spotLight->getPosition())
			* glm::mat4_cast(glm::rotation(defaultDirection, spotLight->getDirection()))
			* glm::scale(glm::vec3(scale, spotLight->getRadius(), scale)));
		m_uSpotLight.set(spotLight);
		m_spotLightMesh->getSubMesh()->render();
	}
}
