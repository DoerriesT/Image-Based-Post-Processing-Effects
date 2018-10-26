#include "StencilRenderPass.h"
#include "Level.h"
#include "Graphics\OpenGL\RenderData.h"
#include <glm\mat4x4.hpp>
#include <glm\vec3.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\ext.hpp>

StencilRenderPass::StencilRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	m_fbo = _fbo;
	m_drawBuffers = { GL_NONE };
	m_state.m_blendState.m_enabled = false;
	m_state.m_blendState.m_sFactor = GL_ONE;
	m_state.m_blendState.m_dFactor = GL_ONE;
	m_state.m_cullFaceState.m_enabled = false;
	m_state.m_cullFaceState.m_face = GL_FRONT;
	m_state.m_depthState.m_enabled = true;
	m_state.m_depthState.m_func = GL_LEQUAL;
	m_state.m_depthState.m_mask = GL_FALSE;
	m_state.m_stencilState.m_enabled = true;
	m_state.m_stencilState.m_frontFunc = m_state.m_stencilState.m_backFunc = GL_ALWAYS;
	m_state.m_stencilState.m_frontRef = m_state.m_stencilState.m_backRef = 1;
	m_state.m_stencilState.m_frontMask = m_state.m_stencilState.m_backMask = 0xFF;
	m_state.m_stencilState.m_frontOpFail = m_state.m_stencilState.m_backOpFail = GL_KEEP;
	m_state.m_stencilState.m_frontOpZfail = GL_DECR_WRAP;
	m_state.m_stencilState.m_backOpZfail = GL_INCR_WRAP;
	m_state.m_stencilState.m_frontOpZpass = m_state.m_stencilState.m_backOpZpass = GL_KEEP;

	resize(_width, _height);

	m_stencilPassShader = ShaderProgram::createShaderProgram("Resources/Shaders/Lighting/lightProxy.vert", "Resources/Shaders/Lighting/stencil.frag");

	m_uModelViewProjection.create(m_stencilPassShader);

	m_pointLightMesh = Mesh::createMesh("Resources/Models/pointlight.mesh", 1, true);
	m_spotLightMesh = Mesh::createMesh("Resources/Models/spotlight.mesh", 1, true);
	m_boxMesh = Mesh::createMesh("Resources/Models/cube.mesh", 1, true);
}

void StencilRenderPass::render(const RenderData & _renderData, const std::shared_ptr<Level>& _level, RenderPass ** _previousRenderPass)
{
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	m_stencilPassShader->bind();

	// point lights
	if (!_level->m_lights.m_pointLights.empty())
	{
		m_pointLightMesh->getSubMesh()->enableVertexAttribArraysPositionOnly();
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

			m_uModelViewProjection.set(_renderData.m_viewProjectionMatrix * glm::translate(pointLight->getPosition()) * glm::scale(glm::vec3(pointLight->getRadius() + 0.1f)));
			m_pointLightMesh->getSubMesh()->render();
		}
	}
	
	// spot lights
	if (!_level->m_lights.m_spotLights.empty())
	{
		m_spotLightMesh->getSubMesh()->enableVertexAttribArraysPositionOnly();
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

			// scale a bit larger to correct for proxy geometry not being exactly round
			float scale = (glm::tan(spotLight->getOuterAngle() * 0.5f) + 0.1f) * spotLight->getRadius();

			const glm::vec3 defaultDirection = glm::vec3(0.0f, -1.0f, 0.0f);

			m_uModelViewProjection.set(_renderData.m_viewProjectionMatrix
				* glm::translate(spotLight->getPosition())
				* glm::mat4_cast(glm::rotation(defaultDirection, spotLight->getDirection()))
				* glm::scale(glm::vec3(scale, spotLight->getRadius(), scale)));
			m_spotLightMesh->getSubMesh()->render();
		}
	}

	// environment probes
	if (!_level->m_environment.m_environmentProbes.empty())
	{
		m_boxMesh->getSubMesh()->enableVertexAttribArraysPositionOnly();
		for (auto probe : _level->m_environment.m_environmentProbes)
		{
			AxisAlignedBoundingBox aabb = probe->getAxisAlignedBoundingBox();
			glm::vec3 boundingBoxCenter = (aabb.m_max + aabb.m_min) * 0.5f;
			glm::vec3 correctedMax = aabb.m_max - boundingBoxCenter;
			glm::vec3 correctedMin = aabb.m_min - boundingBoxCenter;
			glm::vec3 boxScale = correctedMax / 0.5f;

			glm::mat4 modelMatrix = glm::translate(boundingBoxCenter)
				* glm::scale(glm::vec3(boxScale));


			m_uModelViewProjection.set(_renderData.m_viewProjectionMatrix * modelMatrix);
			m_boxMesh->getSubMesh()->render();
		}
	}
	
}
