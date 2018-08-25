#include "LightProbeRenderPass.h"
#include <glm\mat4x4.hpp>
#include <glm\vec3.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\ext.hpp>
#include "Graphics\Scene.h"
#include "Level.h"
#include "Graphics\OpenGL\RenderData.h"
#include "Graphics\EntityRenderData.h"
#include "EntityComponentSystem\Component.h"
#include "Utilities\ContainerUtility.h"
#include "Graphics\Texture.h"
#include "Input\UserInput.h"

LightProbeRenderPass::LightProbeRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	fbo = _fbo;
	drawBuffers = { GL_COLOR_ATTACHMENT4 };
	state.blendState.enabled = false;
	state.blendState.sFactor = GL_SRC_ALPHA;
	state.blendState.dFactor = GL_ONE_MINUS_SRC_ALPHA;
	state.cullFaceState.enabled = true;
	state.cullFaceState.face = GL_BACK;
	state.depthState.enabled = true;
	state.depthState.func = GL_LEQUAL;
	state.depthState.mask = GL_TRUE;
	state.stencilState.enabled = false;
	state.stencilState.frontFunc = state.stencilState.backFunc = GL_ALWAYS;
	state.stencilState.frontRef = state.stencilState.backRef = 1;
	state.stencilState.frontMask = state.stencilState.backMask = 0xFF;
	state.stencilState.frontOpFail = state.stencilState.backOpFail = GL_KEEP;
	state.stencilState.frontOpZfail = state.stencilState.backOpZfail = GL_KEEP;
	state.stencilState.frontOpZpass = state.stencilState.backOpZpass = GL_KEEP;

	resize(_width, _height);

	lightProbeShader = ShaderProgram::createShaderProgram("Resources/Shaders/Renderer/lightProbe.vert", "Resources/Shaders/Renderer/lightProbe.frag");

	uModelViewProjectionMatrix.create(lightProbeShader);

	sphereMesh = Mesh::createMesh("Resources/Models/sphere.mesh", 1, true);
}

void LightProbeRenderPass::render(const RenderData & _renderData, const std::shared_ptr<Level>& _level, RenderPass ** _previousRenderPass)
{
	if (_level->environment.environmentProbes.empty())
	{
		return;
	}
	drawBuffers[0] = _renderData.frame % 2 ? GL_COLOR_ATTACHMENT5 : GL_COLOR_ATTACHMENT4;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	lightProbeShader->bind();

	sphereMesh->getSubMesh()->enableVertexAttribArraysPositionOnly();

	

	for (std::shared_ptr<EnvironmentProbe> environmentProbe : _level->environment.environmentProbes)
	{
		glActiveTexture(GL_TEXTURE13);
		glBindTexture(GL_TEXTURE_2D, environmentProbe->getIrradianceMap()->getId());

		uModelViewProjectionMatrix.set(_renderData.viewProjectionMatrix * glm::translate(environmentProbe->getPosition()) * glm::scale(glm::vec3(0.2f)));

		sphereMesh->getSubMesh()->render();
	}
	
}
