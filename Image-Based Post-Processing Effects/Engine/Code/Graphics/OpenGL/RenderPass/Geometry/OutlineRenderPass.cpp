#include "OutlineRenderPass.h"
#include <glm\mat4x4.hpp>
#include <glm\vec3.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\ext.hpp>
#include "Level.h"
#include "Graphics\OpenGL\RenderData.h"
#include "Graphics\EntityRenderData.h"
#include "Graphics\Scene.h"

OutlineRenderPass::OutlineRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	fbo = _fbo;
	drawBuffers = { GL_COLOR_ATTACHMENT3 };
	state.blendState.enabled = false;
	state.blendState.sFactor = GL_SRC_ALPHA;
	state.blendState.dFactor = GL_ONE_MINUS_SRC_ALPHA;
	state.cullFaceState.enabled = true;
	state.cullFaceState.face = GL_BACK;
	state.depthState.enabled = true;
	state.depthState.func = GL_LEQUAL;
	state.depthState.mask = GL_FALSE;
	state.stencilState.enabled = true;
	state.stencilState.frontFunc = state.stencilState.backFunc = GL_NOTEQUAL;
	state.stencilState.frontRef = state.stencilState.backRef = 1;
	state.stencilState.frontMask = state.stencilState.backMask = 0xFF;
	state.stencilState.frontOpFail = state.stencilState.backOpFail = GL_KEEP;
	state.stencilState.frontOpZfail = state.stencilState.backOpZfail = GL_KEEP;
	state.stencilState.frontOpZpass = state.stencilState.backOpZpass = GL_KEEP;

	resize(_width, _height);

	outlineShader = ShaderProgram::createShaderProgram("Resources/Shaders/Geometry/outline.vert", "Resources/Shaders/Geometry/outline.frag");

	uModelViewProjectionMatrixO.create(outlineShader);
	uOutlineColorO.create(outlineShader);
}

void OutlineRenderPass::render(const RenderData & _renderData, const Scene & _scene, RenderPass **_previousRenderPass)
{
	if (!_scene.getOutlineCount())
	{
		_scene.getOutlineCount();
	}

	drawBuffers[0] = _renderData.frame % 2 ? GL_COLOR_ATTACHMENT4 : GL_COLOR_ATTACHMENT3;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	outlineShader->bind();

	const std::vector<std::unique_ptr<EntityRenderData>> &data = _scene.getData();

	std::shared_ptr<SubMesh> currentMesh = nullptr;
	bool enabledMesh = false;

	for (std::size_t i = 0; i < data.size(); ++i)
	{
		const std::unique_ptr<EntityRenderData> &entityRenderData = data[i];

		// skip this iteration if its supposed to be rendered with another method or does not have sufficient components
		if (!entityRenderData->outlineComponent ||
			!entityRenderData->modelComponent ||
			!entityRenderData->transformationComponent)
		{
			continue;
		}

		if (currentMesh != entityRenderData->mesh)
		{
			currentMesh = entityRenderData->mesh;
			enabledMesh = false;
		}

		// skip this iteration if the mesh is not yet valid
		if (!currentMesh || !currentMesh->isValid())
		{
			continue;
		}

		if (!enabledMesh)
		{
			enabledMesh = true;
			currentMesh->enableVertexAttribArraysPositionOnly();
		}

		// we're good to go: render this mesh-entity instance
		glm::mat4 modelMatrix = glm::translate(entityRenderData->transformationComponent->position)
			* glm::mat4_cast(entityRenderData->transformationComponent->rotation)
			* glm::scale(glm::vec3(entityRenderData->transformationComponent->scale * entityRenderData->outlineComponent->scaleMultiplier));

		uOutlineColorO.set(entityRenderData->outlineComponent->outlineColor);
		uModelViewProjectionMatrixO.set(_renderData.viewProjectionMatrix * modelMatrix);

		currentMesh->render();
	}
}
