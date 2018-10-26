#include "SkyboxRenderPass.h"
#include <glm\mat4x4.hpp>
#include <glm\vec3.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\ext.hpp>
#include "Level.h"
#include "Graphics\Texture.h"
#include "Graphics\OpenGL\RenderData.h"
#include "EntityComponentSystem\EntityManager.h"

SkyboxRenderPass::SkyboxRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	m_fbo = _fbo;
	m_drawBuffers = { GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT3 };
	m_state.m_blendState.m_enabled = false;
	m_state.m_cullFaceState.m_enabled = false;
	m_state.m_cullFaceState.m_face = GL_BACK;
	m_state.m_depthState.m_enabled = true;
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

	m_skyboxShader = ShaderProgram::createShaderProgram("Resources/Shaders/Geometry/skybox.vert", "Resources/Shaders/Geometry/skybox.frag");

	m_uInverseModelViewProjection.create(m_skyboxShader);
	m_uColor.create(m_skyboxShader);
	m_uHasAlbedoMap.create(m_skyboxShader);
	m_uCurrentToPrevTransform.create(m_skyboxShader);

	m_fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void SkyboxRenderPass::render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, RenderPass **_previousRenderPass)
{
	Environment &environment = _level->m_environment;
	if (!environment.m_skyboxEntity)
	{
		return;
	}

	m_drawBuffers[0] = _renderData.m_frame % 2 ? GL_COLOR_ATTACHMENT5 : GL_COLOR_ATTACHMENT4;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	const glm::vec4 DEFAULT_ALBEDO_COLOR(1.0);
	static glm::mat4 prevTransform;

	m_fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();
	m_skyboxShader->bind();

	EntityManager &entityManager = EntityManager::getInstance();

	m_uHasAlbedoMap.set(environment.m_environmentMap ? true : false);
	m_uColor.set(DEFAULT_ALBEDO_COLOR);

	if (environment.m_environmentMap)
	{
		glActiveTexture(GL_TEXTURE11);
		glBindTexture(environment.m_environmentMap->getTarget(), environment.m_environmentMap->getId());
	}

	TransformationComponent *transformationComponent = entityManager.getComponent<TransformationComponent>(environment.m_skyboxEntity);
	glm::mat4 mvpMatrix = _renderData.m_projectionMatrix * glm::mat4(glm::mat3(_renderData.m_viewMatrix));

	if (transformationComponent)
	{
		mvpMatrix *= glm::mat4_cast(transformationComponent->m_rotation);
	}

	glm::mat4 invTransform = glm::inverse(mvpMatrix);

	m_uInverseModelViewProjection.set(invTransform);
	m_uCurrentToPrevTransform.set(prevTransform * invTransform);

	prevTransform = mvpMatrix;

	m_fullscreenTriangle->getSubMesh()->render();
}
