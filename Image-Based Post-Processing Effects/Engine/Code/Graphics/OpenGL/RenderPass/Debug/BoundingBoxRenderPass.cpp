#include "BoundingBoxRenderPass.h"
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

BoundingBoxRenderPass::BoundingBoxRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	fbo = _fbo;
	drawBuffers = { GL_COLOR_ATTACHMENT0 };
	state.blendState.enabled = false;
	state.blendState.sFactor = GL_SRC_ALPHA;
	state.blendState.dFactor = GL_ONE_MINUS_SRC_ALPHA;
	state.cullFaceState.enabled = false;
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

	boundingBoxShader = ShaderProgram::createShaderProgram("Resources/Shaders/Debug/boundingBox.vert", "Resources/Shaders/Debug/boundingBox.frag");

	uModelViewProjectionMatrix.create(boundingBoxShader);
	uColor.create(boundingBoxShader);

	boxMesh = Mesh::createMesh("Resources/Models/cube.mesh", 1, true);
}

void BoundingBoxRenderPass::render(const RenderData & _renderData, const std::shared_ptr<Level>& _level, const Scene & _scene, RenderPass ** _previousRenderPass)
{
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	boundingBoxShader->bind();

	boxMesh->getSubMesh()->enableVertexAttribArraysPositionOnly();

	uColor.set(glm::vec3(1.0f, 0.0f, 0.0f));

	const std::vector<std::unique_ptr<EntityRenderData>> &data = _scene.getData();

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//for (std::size_t i = 0; i < data.size(); ++i)
	//{
	//	const std::unique_ptr<EntityRenderData> &entityRenderData = data[i];
	//
	//	if (!entityRenderData->modelComponent || !entityRenderData->transformationComponent)
	//	{
	//		continue;
	//	}
	//
	//	// skip this iteration if the mesh is not yet valid
	//	if (!entityRenderData->mesh->isValid())
	//	{
	//		continue;
	//	}
	//
	//	AxisAlignedBoundingBox aabb = entityRenderData->mesh->getAABB();
	//	glm::vec3 boundingBoxCenter = (aabb.max + aabb.min) * 0.5f;
	//	glm::vec3 correctedMax = aabb.max - boundingBoxCenter;
	//	glm::vec3 correctedMin = aabb.min - boundingBoxCenter;
	//	glm::vec3 boxScale = correctedMax / 0.5f;
	//
	//
	//	glm::mat4 modelMatrix = glm::translate(entityRenderData->transformationComponent->position + boundingBoxCenter)
	//		* glm::mat4_cast(entityRenderData->transformationComponent->rotation)
	//		* glm::scale(glm::vec3(entityRenderData->transformationComponent->scale * boxScale));
	//
	//	
	//	uModelViewProjectionMatrix.set(_renderData.viewProjectionMatrix * modelMatrix);
	//	
	//
	//	boxMesh->getSubMesh()->render();
	//}

	for (const auto &probe : _level->environment.environmentProbes)
	{
		AxisAlignedBoundingBox aabb = probe->getAxisAlignedBoundingBox();
		glm::vec3 boundingBoxCenter = (aabb.max + aabb.min) * 0.5f;
		glm::vec3 correctedMax = aabb.max - boundingBoxCenter;
		glm::vec3 correctedMin = aabb.min - boundingBoxCenter;
		glm::vec3 boxScale = correctedMax / 0.5f;

		glm::mat4 modelMatrix = glm::translate(boundingBoxCenter)
			* glm::scale(glm::vec3(boxScale));


		uModelViewProjectionMatrix.set(_renderData.viewProjectionMatrix * modelMatrix);

		boxMesh->getSubMesh()->render();
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
