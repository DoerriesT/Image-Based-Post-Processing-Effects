#include "PointLightRenderPass.h"
#include <glm\mat4x4.hpp>
#include <glm\vec3.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\ext.hpp>
#include "Level.h"
#include "Graphics\OpenGL\RenderData.h"

PointLightRenderPass::PointLightRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	fbo = _fbo;
	drawBuffers = { GL_COLOR_ATTACHMENT4 };
	state.blendState.enabled = true;
	state.blendState.sFactor = GL_ONE;
	state.blendState.dFactor = GL_ONE;
	state.cullFaceState.enabled = true;
	state.cullFaceState.face = GL_FRONT;
	state.depthState.enabled = false;
	state.depthState.func = GL_LEQUAL;
	state.depthState.mask = GL_FALSE;
	state.stencilState.enabled = true;
	state.stencilState.frontFunc = state.stencilState.backFunc = GL_NOTEQUAL;
	state.stencilState.frontRef = state.stencilState.backRef = 0;
	state.stencilState.frontMask = state.stencilState.backMask = 0xFF;
	state.stencilState.frontOpFail = state.stencilState.backOpFail = GL_KEEP;
	state.stencilState.frontOpZfail = state.stencilState.backOpZfail = GL_KEEP;
	state.stencilState.frontOpZpass = state.stencilState.backOpZpass = GL_KEEP;

	resize(_width, _height);

	pointLightPassShader = ShaderProgram::createShaderProgram("Resources/Shaders/Lighting/lightProxy.vert", "Resources/Shaders/Lighting/pointLight.frag");

	uModelViewProjectionP.create(pointLightPassShader);
	uPointLightP.create(pointLightPassShader);
	uInverseProjectionP.create(pointLightPassShader);
	uInverseViewP.create(pointLightPassShader);
	uShadowsEnabledP.create(pointLightPassShader);
	uViewportSizeP.create(pointLightPassShader);

	pointLightMesh = Mesh::createMesh("Resources/Models/pointlight.mesh", 1, true);
}

void PointLightRenderPass::render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const GBuffer &_gbuffer, RenderPass **_previousRenderPass)
{
	if (_level->lights.pointLights.empty())
	{
		return;
	}

	drawBuffers[0] = _renderData.frame % 2 ? GL_COLOR_ATTACHMENT5 : GL_COLOR_ATTACHMENT4;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	pointLightMesh->getSubMesh()->enableVertexAttribArrays();

	pointLightPassShader->bind();

	uInverseViewP.set(_renderData.invViewMatrix);
	uInverseProjectionP.set(_renderData.invProjectionMatrix);
	uShadowsEnabledP.set(_renderData.shadows);
	uViewportSizeP.set(glm::vec2(_renderData.resolution.first, _renderData.resolution.second));

	for (std::shared_ptr<PointLight> pointLight : _level->lights.pointLights)
	{
		if (!_renderData.frustum.testSphere(pointLight->getBoundingSphere()))
		{
			continue;
		}

		pointLight->updateViewValues(_renderData.viewMatrix);

		if (pointLight->isRenderShadows())
		{
			glActiveTexture(GL_TEXTURE14);
			glBindTexture(GL_TEXTURE_CUBE_MAP, pointLight->getShadowMap());
		}

		uModelViewProjectionP.set(_renderData.viewProjectionMatrix * glm::translate(pointLight->getPosition()) * glm::scale(glm::vec3(pointLight->getRadius() + 0.1f)));
		uPointLightP.set(pointLight);
		pointLightMesh->getSubMesh()->render();
	}
}
