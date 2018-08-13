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
	fbo = _fbo;
	drawBuffers = { GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT3 };
	state.blendState.enabled = false;
	state.cullFaceState.enabled = false;
	state.cullFaceState.face = GL_BACK;
	state.depthState.enabled = true;
	state.depthState.func = GL_LEQUAL;
	state.depthState.mask = GL_FALSE;
	state.stencilState.enabled = false;
	state.stencilState.frontFunc = state.stencilState.backFunc = GL_ALWAYS;
	state.stencilState.frontRef = state.stencilState.backRef = 1;
	state.stencilState.frontMask = state.stencilState.backMask = 0xFF;
	state.stencilState.frontOpFail = state.stencilState.backOpFail = GL_KEEP;
	state.stencilState.frontOpZfail = state.stencilState.backOpZfail = GL_KEEP;
	state.stencilState.frontOpZpass = state.stencilState.backOpZpass = GL_KEEP;

	resize(_width, _height);

	skyboxShader = ShaderProgram::createShaderProgram("Resources/Shaders/Renderer/skybox.vert", "Resources/Shaders/Renderer/skybox.frag");

	uInverseModelViewProjectionB.create(skyboxShader);
	uColorB.create(skyboxShader);
	uHasAlbedoMapB.create(skyboxShader);
	uCurrentToPrevTransformB.create(skyboxShader);

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void SkyboxRenderPass::render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, RenderPass **_previousRenderPass)
{
	if (!_level->environment.skyboxEntity)
	{
		return;
	}

	drawBuffers[0] = _renderData.frame % 2 ? GL_COLOR_ATTACHMENT5 : GL_COLOR_ATTACHMENT4;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	const glm::vec4 DEFAULT_ALBEDO_COLOR(1.0);
	static glm::mat4 prevTransform;

	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();
	skyboxShader->bind();

	EntityManager &entityManager = EntityManager::getInstance();

	uHasAlbedoMapB.set(_level->environment.environmentMap ? true : false);
	uColorB.set(DEFAULT_ALBEDO_COLOR);

	if (_level->environment.environmentMap)
	{
		glActiveTexture(GL_TEXTURE11);
		glBindTexture(_level->environment.environmentMap->getTarget(), _level->environment.environmentMap->getId());
	}

	TransformationComponent *transformationComponent = entityManager.getComponent<TransformationComponent>(_level->environment.skyboxEntity);
	glm::mat4 mvpMatrix = _renderData.projectionMatrix * glm::mat4(glm::mat3(_renderData.viewMatrix));

	if (transformationComponent)
	{
		mvpMatrix *= glm::mat4_cast(transformationComponent->rotation);
	}

	glm::mat4 invTransform = glm::inverse(mvpMatrix);

	uInverseModelViewProjectionB.set(invTransform);
	uCurrentToPrevTransformB.set(prevTransform * invTransform);

	prevTransform = mvpMatrix;

	fullscreenTriangle->getSubMesh()->render();
}
