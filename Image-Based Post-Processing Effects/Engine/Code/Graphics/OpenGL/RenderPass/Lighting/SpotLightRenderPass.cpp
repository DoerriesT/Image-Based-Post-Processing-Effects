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

	spotLightPassShader = ShaderProgram::createShaderProgram("Resources/Shaders/Lighting/lightProxy.vert", "Resources/Shaders/Lighting/spotLight.frag");

	uModelViewProjectionS.create(spotLightPassShader);
	uSpotLightS.create(spotLightPassShader);
	uInverseViewS.create(spotLightPassShader);
	uInverseProjectionS.create(spotLightPassShader);
	uShadowsEnabledS.create(spotLightPassShader);
	uViewportSizeS.create(spotLightPassShader);

	spotLightMesh = Mesh::createMesh("Resources/Models/spotlight.mesh", 1, true);
}

void SpotLightRenderPass::render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const GBuffer &_gbuffer, RenderPass **_previousRenderPass)
{
	if (_level->lights.spotLights.empty())
	{
		return;
	}

	drawBuffers[0] = _renderData.frame % 2 ? GL_COLOR_ATTACHMENT5 : GL_COLOR_ATTACHMENT4;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	spotLightMesh->getSubMesh()->enableVertexAttribArrays();

	spotLightPassShader->bind();

	uInverseViewS.set(_renderData.invViewMatrix);
	uInverseProjectionS.set(_renderData.invProjectionMatrix);
	uShadowsEnabledS.set(_renderData.shadows);
	uViewportSizeS.set(glm::vec2(_renderData.resolution.first, _renderData.resolution.second));

	for (std::shared_ptr<SpotLight> spotLight : _level->lights.spotLights)
	{
		if (!_renderData.frustum.testSphere(spotLight->getBoundingSphere()))
		{
			continue;
		}

		spotLight->updateViewValues(_renderData.viewMatrix);

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
		float scale = (glm::tan(spotLight->getOuterAngle()) + 0.1f) * spotLight->getRadius();

		const glm::vec3 defaultDirection = glm::vec3(0.0f, -1.0f, 0.0f);

		uModelViewProjectionS.set(_renderData.viewProjectionMatrix
			* glm::translate(spotLight->getPosition())
			* glm::mat4_cast(glm::rotation(defaultDirection, spotLight->getDirection()))
			* glm::scale(glm::vec3(scale, spotLight->getRadius(), scale)));
		uSpotLightS.set(spotLight);
		spotLightMesh->getSubMesh()->render();
	}
}
