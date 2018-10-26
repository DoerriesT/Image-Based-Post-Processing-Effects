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
	m_fbo = _fbo;
	m_drawBuffers = { GL_COLOR_ATTACHMENT4 };
	m_state.m_blendState.m_enabled = false;
	m_state.m_blendState.m_sFactor = GL_SRC_ALPHA;
	m_state.m_blendState.m_dFactor = GL_ONE_MINUS_SRC_ALPHA;
	m_state.m_cullFaceState.m_enabled = true;
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

	m_lightProbeShader = ShaderProgram::createShaderProgram("Resources/Shaders/Geometry/lightProbe.vert", "Resources/Shaders/Geometry/lightProbe.frag");

	m_uModelViewProjectionMatrix.create(m_lightProbeShader);
	m_uSH.create(m_lightProbeShader);
	m_uIndex.create(m_lightProbeShader);

	m_sphereMesh = Mesh::createMesh("Resources/Models/sphere.mesh", 1, true);
}

extern bool renderLightProbes;

void LightProbeRenderPass::render(const RenderData & _renderData, const std::shared_ptr<Level>& _level, RenderPass ** _previousRenderPass)
{
	if (!_level->m_environment.m_irradianceVolume || !renderLightProbes)
	{
		return;
	}
	m_drawBuffers[0] = _renderData.m_frame % 2 ? GL_COLOR_ATTACHMENT5 : GL_COLOR_ATTACHMENT4;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	m_lightProbeShader->bind();

	m_sphereMesh->getSubMesh()->enableVertexAttribArraysPositionOnly();

	std::shared_ptr<IrradianceVolume> volume = _level->m_environment.m_irradianceVolume;
	glm::ivec3 dims = volume->getDimensions();
	glm::vec3 origin = volume->getOrigin();
	float spacing = volume->getSpacing();

	glActiveTexture(GL_TEXTURE13);
	glBindTexture(GL_TEXTURE_2D, volume->getProbeTexture()->getId());

	for (int z = 0; z < dims.z; ++z)
	{
		for (int y = 0; y < dims.y; ++y)
		{
			for (int x = 0; x < dims.x; ++x)
			{
				glm::vec3 position = origin + glm::vec3(x, y, z) * spacing;
				m_uModelViewProjectionMatrix.set(_renderData.m_viewProjectionMatrix * glm::translate(position) * glm::scale(glm::vec3(0.2f)));
				int index = z * (dims.x * dims.y) + y * dims.x + x;
				m_uIndex.set(index);

				auto data = volume->getProbeData({x, y, z});

				m_sphereMesh->getSubMesh()->render();
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
