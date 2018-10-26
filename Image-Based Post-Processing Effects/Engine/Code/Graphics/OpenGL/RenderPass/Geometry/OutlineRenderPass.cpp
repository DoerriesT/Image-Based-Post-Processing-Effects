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
	m_fbo = _fbo;
	m_drawBuffers = { GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT3 };
	m_state.m_blendState.m_enabled = false;
	m_state.m_blendState.m_sFactor = GL_SRC_ALPHA;
	m_state.m_blendState.m_dFactor = GL_ONE_MINUS_SRC_ALPHA;
	m_state.m_cullFaceState.m_enabled = true;
	m_state.m_cullFaceState.m_face = GL_BACK;
	m_state.m_depthState.m_enabled = true;
	m_state.m_depthState.m_func = GL_LEQUAL;
	m_state.m_depthState.m_mask = GL_FALSE;
	m_state.m_stencilState.m_enabled = true;
	m_state.m_stencilState.m_frontFunc = m_state.m_stencilState.m_backFunc = GL_NOTEQUAL;
	m_state.m_stencilState.m_frontRef = m_state.m_stencilState.m_backRef = 1;
	m_state.m_stencilState.m_frontMask = m_state.m_stencilState.m_backMask = 0xFF;
	m_state.m_stencilState.m_frontOpFail = m_state.m_stencilState.m_backOpFail = GL_KEEP;
	m_state.m_stencilState.m_frontOpZfail = m_state.m_stencilState.m_backOpZfail = GL_KEEP;
	m_state.m_stencilState.m_frontOpZpass = m_state.m_stencilState.m_backOpZpass = GL_KEEP;

	resize(_width, _height);

	m_outlineShader = ShaderProgram::createShaderProgram("Resources/Shaders/Geometry/outline.vert", "Resources/Shaders/Geometry/outline.frag");

	m_uModelViewProjectionMatrix.create(m_outlineShader);
	m_uOutlineColor.create(m_outlineShader);
}

void OutlineRenderPass::render(const RenderData & _renderData, const Scene & _scene, RenderPass **_previousRenderPass)
{
	if (!_scene.getOutlineCount())
	{
		_scene.getOutlineCount();
	}

	m_drawBuffers[0] = _renderData.m_frame % 2 ? GL_COLOR_ATTACHMENT5 : GL_COLOR_ATTACHMENT4;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	m_outlineShader->bind();

	const std::vector<std::unique_ptr<EntityRenderData>> &data = _scene.getData();

	std::shared_ptr<SubMesh> currentMesh = nullptr;
	bool enabledMesh = false;

	for (std::size_t i = 0; i < data.size(); ++i)
	{
		const std::unique_ptr<EntityRenderData> &entityRenderData = data[i];

		// continue if this is a bake and the entity is not static
		if (entityRenderData->m_transformationComponent && entityRenderData->m_transformationComponent->m_mobility != Mobility::STATIC && _renderData.m_bake)
		{
			continue;
		}

		// skip this iteration if its supposed to be rendered with another method or does not have sufficient components
		if (!entityRenderData->m_outlineComponent ||
			!entityRenderData->m_modelComponent ||
			!entityRenderData->m_transformationComponent)
		{
			continue;
		}

		if (currentMesh != entityRenderData->m_mesh)
		{
			currentMesh = entityRenderData->m_mesh;
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

		m_uOutlineColor.set(entityRenderData->m_outlineComponent->m_outlineColor);
		m_uModelViewProjectionMatrix.set(_renderData.m_viewProjectionMatrix * entityRenderData->m_transformationComponent->m_transformation);

		currentMesh->render();
	}
}
