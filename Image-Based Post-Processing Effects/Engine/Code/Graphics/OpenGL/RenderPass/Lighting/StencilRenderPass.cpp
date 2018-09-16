#include "StencilRenderPass.h"
#include "Level.h"
#include "Graphics\OpenGL\RenderData.h"
#include <glm\mat4x4.hpp>
#include <glm\vec3.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\ext.hpp>

StencilRenderPass::StencilRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	fbo = _fbo;
	drawBuffers = { GL_NONE };
	state.blendState.enabled = false;
	state.blendState.sFactor = GL_ONE;
	state.blendState.dFactor = GL_ONE;
	state.cullFaceState.enabled = false;
	state.cullFaceState.face = GL_FRONT;
	state.depthState.enabled = true;
	state.depthState.func = GL_LEQUAL;
	state.depthState.mask = GL_FALSE;
	state.stencilState.enabled = true;
	state.stencilState.frontFunc = state.stencilState.backFunc = GL_ALWAYS;
	state.stencilState.frontRef = state.stencilState.backRef = 1;
	state.stencilState.frontMask = state.stencilState.backMask = 0xFF;
	state.stencilState.frontOpFail = state.stencilState.backOpFail = GL_KEEP;
	state.stencilState.frontOpZfail = GL_DECR_WRAP;
	state.stencilState.backOpZfail = GL_INCR_WRAP;
	state.stencilState.frontOpZpass = state.stencilState.backOpZpass = GL_KEEP;

	resize(_width, _height);

	stencilPassShader = ShaderProgram::createShaderProgram("Resources/Shaders/Lighting/lightProxy.vert", "Resources/Shaders/Lighting/stencil.frag");

	uModelViewProjection.create(stencilPassShader);

	pointLightMesh = Mesh::createMesh("Resources/Models/pointlight.mesh", 1, true);
	spotLightMesh = Mesh::createMesh("Resources/Models/spotlight.mesh", 1, true);
	boxMesh = Mesh::createMesh("Resources/Models/cube.mesh", 1, true);
}

void StencilRenderPass::render(const RenderData & _renderData, const std::shared_ptr<Level>& _level, const GBuffer & _gbuffer, RenderPass ** _previousRenderPass)
{
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	stencilPassShader->bind();

	// point lights
	if (!_level->lights.pointLights.empty())
	{
		pointLightMesh->getSubMesh()->enableVertexAttribArraysPositionOnly();
		for (std::shared_ptr<PointLight> pointLight : _level->lights.pointLights)
		{
			if (_renderData.bake && pointLight->getMobility() != Mobility::STATIC)
			{
				continue;
			}

			if (!_renderData.frustum.testSphere(pointLight->getBoundingSphere()))
			{
				continue;
			}

			pointLight->updateViewValues(_renderData.viewMatrix);

			uModelViewProjection.set(_renderData.viewProjectionMatrix * glm::translate(pointLight->getPosition()) * glm::scale(glm::vec3(pointLight->getRadius() + 0.1f)));
			pointLightMesh->getSubMesh()->render();
		}
	}
	
	// spot lights
	if (!_level->lights.spotLights.empty())
	{
		spotLightMesh->getSubMesh()->enableVertexAttribArraysPositionOnly();
		for (std::shared_ptr<SpotLight> spotLight : _level->lights.spotLights)
		{
			if (_renderData.bake && spotLight->getMobility() != Mobility::STATIC)
			{
				continue;
			}

			if (!_renderData.frustum.testSphere(spotLight->getBoundingSphere()))
			{
				continue;
			}

			spotLight->updateViewValues(_renderData.viewMatrix);

			// scale a bit larger to correct for proxy geometry not being exactly round
			float scale = (glm::tan(spotLight->getOuterAngle() * 0.5f) + 0.1f) * spotLight->getRadius();

			const glm::vec3 defaultDirection = glm::vec3(0.0f, -1.0f, 0.0f);

			uModelViewProjection.set(_renderData.viewProjectionMatrix
				* glm::translate(spotLight->getPosition())
				* glm::mat4_cast(glm::rotation(defaultDirection, spotLight->getDirection()))
				* glm::scale(glm::vec3(scale, spotLight->getRadius(), scale)));
			spotLightMesh->getSubMesh()->render();
		}
	}

	// environment probes
	if (!_level->environment.environmentProbes.empty())
	{
		boxMesh->getSubMesh()->enableVertexAttribArraysPositionOnly();
		for (auto probe : _level->environment.environmentProbes)
		{
			AxisAlignedBoundingBox aabb = probe->getAxisAlignedBoundingBox();
			glm::vec3 boundingBoxCenter = (aabb.max + aabb.min) * 0.5f;
			glm::vec3 correctedMax = aabb.max - boundingBoxCenter;
			glm::vec3 correctedMin = aabb.min - boundingBoxCenter;
			glm::vec3 boxScale = correctedMax / 0.5f;

			glm::mat4 modelMatrix = glm::translate(boundingBoxCenter)
				* glm::scale(glm::vec3(boxScale));


			uModelViewProjection.set(_renderData.viewProjectionMatrix * modelMatrix);
			boxMesh->getSubMesh()->render();
		}
	}
	
}
