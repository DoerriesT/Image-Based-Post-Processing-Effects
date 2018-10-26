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
	m_fbo = _fbo;
	m_drawBuffers = { GL_COLOR_ATTACHMENT0 };
	m_state.m_blendState.m_enabled = false;
	m_state.m_blendState.m_sFactor = GL_SRC_ALPHA;
	m_state.m_blendState.m_dFactor = GL_ONE_MINUS_SRC_ALPHA;
	m_state.m_cullFaceState.m_enabled = false;
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

	m_boundingBoxShader = ShaderProgram::createShaderProgram("Resources/Shaders/Debug/boundingBox.vert", "Resources/Shaders/Debug/boundingBox.frag");

	m_uModelViewProjectionMatrix.create(m_boundingBoxShader);
	m_uColor.create(m_boundingBoxShader);

	boxMesh = Mesh::createMesh("Resources/Models/cube.mesh", 1, true);
}

void BoundingBoxRenderPass::render(const RenderData & _renderData, const std::shared_ptr<Level>& _level, const Scene & _scene, RenderPass ** _previousRenderPass)
{
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	m_boundingBoxShader->bind();

	boxMesh->getSubMesh()->enableVertexAttribArraysPositionOnly();

	m_uColor.set(glm::vec3(1.0f, 0.0f, 0.0f));

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

	for (const auto &probe : _level->m_environment.m_environmentProbes)
	{
		AxisAlignedBoundingBox aabb = probe->getAxisAlignedBoundingBox();
		glm::vec3 boundingBoxCenter = (aabb.m_max + aabb.m_min) * 0.5f;
		glm::vec3 correctedMax = aabb.m_max - boundingBoxCenter;
		glm::vec3 correctedMin = aabb.m_min - boundingBoxCenter;
		glm::vec3 boxScale = correctedMax / 0.5f;

		glm::mat4 modelMatrix = glm::translate(boundingBoxCenter)
			* glm::scale(glm::vec3(boxScale));


		m_uModelViewProjectionMatrix.set(_renderData.m_viewProjectionMatrix * modelMatrix);

		boxMesh->getSubMesh()->render();
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
