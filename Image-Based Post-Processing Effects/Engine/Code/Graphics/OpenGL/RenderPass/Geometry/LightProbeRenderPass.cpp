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

	lightProbeShader = ShaderProgram::createShaderProgram("Resources/Shaders/Geometry/lightProbe.vert", "Resources/Shaders/Geometry/lightProbe.frag");

	uModelViewProjectionMatrix.create(lightProbeShader);
	uSH.create(lightProbeShader);
	uIndex.create(lightProbeShader);

	sphereMesh = Mesh::createMesh("Resources/Models/sphere.mesh", 1, true);
}

extern bool renderLightProbes;

void LightProbeRenderPass::render(const RenderData & _renderData, const std::shared_ptr<Level>& _level, RenderPass ** _previousRenderPass)
{
	if (!_level->environment.irradianceVolume || !renderLightProbes)
	{
		return;
	}
	drawBuffers[0] = _renderData.frame % 2 ? GL_COLOR_ATTACHMENT5 : GL_COLOR_ATTACHMENT4;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	lightProbeShader->bind();

	sphereMesh->getSubMesh()->enableVertexAttribArraysPositionOnly();

	glm::ivec3 dims = _level->environment.irradianceVolume->getDimensions();
	glm::vec3 origin = _level->environment.irradianceVolume->getOrigin();
	float spacing = _level->environment.irradianceVolume->getSpacing();

	glActiveTexture(GL_TEXTURE13);
	glBindTexture(GL_TEXTURE_2D, _level->environment.irradianceVolume->getProbeTexture()->getId());

	for (unsigned int z = 0; z < dims.z; ++z)
	{
		for (unsigned int y = 0; y < dims.y; ++y)
		{
			for (unsigned int x = 0; x < dims.x; ++x)
			{
				glm::vec3 position = origin + glm::vec3(x, y, z) * spacing;
				uModelViewProjectionMatrix.set(_renderData.viewProjectionMatrix * glm::translate(position) * glm::scale(glm::vec3(0.2f)));
				int index = z * (dims.x * dims.y) + y * dims.x + x;
				uIndex.set(index);

				auto data = _level->environment.irradianceVolume->getProbeData({x, y, z});

				sphereMesh->getSubMesh()->render();
			}
		}
	}

	//for (std::shared_ptr<EnvironmentProbe> environmentProbe : _level->environment.environmentProbes)
	//{
	//	glActiveTexture(GL_TEXTURE13);
	//	glBindTexture(GL_TEXTURE_2D, environmentProbe->getIrradianceTexture()->getId());
	//
	//	uModelViewProjectionMatrix.set(_renderData.viewProjectionMatrix * glm::translate(environmentProbe->getPosition()) * glm::scale(glm::vec3(0.2f)));
	//	uSH.set(UserInput::getInstance().isKeyPressed(InputKey::H));
	//
	//	sphereMesh->getSubMesh()->render();
	//}
	
}
