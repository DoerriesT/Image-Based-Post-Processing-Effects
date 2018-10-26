#include "ForwardRenderPass.h"
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

ForwardRenderPass::ForwardRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	m_fbo = _fbo;
	m_drawBuffers = { GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT3 };
	m_state.m_blendState.m_enabled = true;
	m_state.m_blendState.m_sFactor = GL_SRC_ALPHA;
	m_state.m_blendState.m_dFactor = GL_ONE_MINUS_SRC_ALPHA;
	m_state.m_cullFaceState.m_enabled = true;
	m_state.m_cullFaceState.m_face = GL_BACK;
	m_state.m_depthState.m_enabled = true;
	m_state.m_depthState.m_func = GL_LEQUAL;
	m_state.m_depthState.m_mask = GL_TRUE;
	m_state.m_stencilState.m_enabled = true;
	m_state.m_stencilState.m_frontFunc = m_state.m_stencilState.m_backFunc = GL_ALWAYS;
	m_state.m_stencilState.m_frontRef = m_state.m_stencilState.m_backRef = 1;
	m_state.m_stencilState.m_frontMask = m_state.m_stencilState.m_backMask = 0xFF;
	m_state.m_stencilState.m_frontOpFail = m_state.m_stencilState.m_backOpFail = GL_KEEP;
	m_state.m_stencilState.m_frontOpZfail = m_state.m_stencilState.m_backOpZfail = GL_KEEP;
	m_state.m_stencilState.m_frontOpZpass = m_state.m_stencilState.m_backOpZpass = GL_KEEP;

	resize(_width, _height);

	m_forwardShader = ShaderProgram::createShaderProgram("Resources/Shaders/Geometry/forward.vert", "Resources/Shaders/Geometry/forward.frag");

	m_uViewMatrix.create(m_forwardShader);
	m_uPrevTransform.create(m_forwardShader);
	m_uModelViewProjectionMatrix.create(m_forwardShader);
	m_uModelMatrix.create(m_forwardShader);
	m_uAtlasData.create(m_forwardShader);
	m_uMaterial.create(m_forwardShader);
	m_uDirectionalLight.create(m_forwardShader);
	m_uRenderDirectionalLight.create(m_forwardShader);
	m_uCamPos.create(m_forwardShader);
	m_uShadowsEnabled.create(m_forwardShader);
	m_uCurrTransform.create(m_forwardShader);
}

void ForwardRenderPass::render(const RenderData & _renderData, const std::shared_ptr<Level>& _level, const Scene & _scene, RenderPass **_previousRenderPass)
{
	if (!_scene.getTransparencyCount())
	{
		return;
	}

	m_drawBuffers[0] = _renderData.m_frame % 2 ? GL_COLOR_ATTACHMENT5 : GL_COLOR_ATTACHMENT4;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	m_forwardShader->bind();

	if (!_level->m_lights.m_directionalLights.empty())
	{
		if (_level->m_lights.m_directionalLights[0]->isRenderShadows())
		{
			glActiveTexture(GL_TEXTURE15);
			glBindTexture(GL_TEXTURE_2D_ARRAY, _level->m_lights.m_directionalLights[0]->getShadowMap());
		}
		m_uRenderDirectionalLight.set(true);
		m_uDirectionalLight.set(_level->m_lights.m_directionalLights[0]);
	}
	else
	{
		m_uRenderDirectionalLight.set(false);
	}
	m_uShadowsEnabled.set(_renderData.m_shadows);
	m_uCamPos.set(_renderData.m_cameraPosition);
	m_uViewMatrix.set(_renderData.m_viewMatrix);

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
		if (entityRenderData->m_customOpaqueShaderComponent ||
			entityRenderData->m_customTransparencyShaderComponent ||
			!entityRenderData->m_transparencyComponent ||
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

		// skip this mesh if its not transparent
		if (entityRenderData->m_transparencyComponent && !ContainerUtility::contains(entityRenderData->m_transparencyComponent->m_transparentSubMeshes, currentMesh))
		{
			continue;
		}

		// skip this iteration if the mesh is not yet valid
		if (!currentMesh || !currentMesh->isValid())
		{
			continue;
		}

		if (!enabledMesh)
		{
			enabledMesh = true;
			currentMesh->enableVertexAttribArrays();
		}

		// we're good to go: render this mesh-entity instance
		m_uMaterial.set(entityRenderData->m_material);
		entityRenderData->m_material->bindTextures();

		int rows = 1;
		int columns = 1;
		glm::vec2 textureOffset;
		TextureAtlasIndexComponent *textureAtlasComponent = entityRenderData->m_textureAtlasIndexComponent;
		if (textureAtlasComponent && ContainerUtility::contains(textureAtlasComponent->m_meshToIndexMap, currentMesh))
		{
			rows = textureAtlasComponent->m_rows;
			columns = textureAtlasComponent->m_columns;
			int texPos = textureAtlasComponent->m_meshToIndexMap[currentMesh];
			int col = texPos % columns;
			int row = texPos / columns;
			textureOffset = glm::vec2((float)col / columns, (float)row / rows);
		}

		glm::mat4 transformation = _renderData.m_viewProjectionMatrix * entityRenderData->m_transformationComponent->m_transformation;
		glm::mat4 prevTransformation = _renderData.m_prevViewProjectionMatrix * entityRenderData->m_transformationComponent->m_prevTransformation;

		m_uAtlasData.set(glm::vec4(1.0f / columns, 1.0f / rows, textureOffset));
		m_uModelMatrix.set(entityRenderData->m_transformationComponent->m_transformation);
		m_uModelViewProjectionMatrix.set(transformation);
		m_uPrevTransform.set(_renderData.m_prevInvJitter * prevTransformation);
		m_uCurrTransform.set(_renderData.m_invJitter * transformation);

		if (entityRenderData->m_outlineComponent)
		{
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
			glStencilMask(0xFF);
		}

		currentMesh->render();

		if (entityRenderData->m_outlineComponent)
		{
			glStencilMaskSeparate(GL_FRONT, m_state.m_stencilState.m_frontMask);
			glStencilMaskSeparate(GL_BACK, m_state.m_stencilState.m_backMask);
			glStencilOpSeparate(GL_FRONT, m_state.m_stencilState.m_frontOpFail, m_state.m_stencilState.m_frontOpZfail, m_state.m_stencilState.m_frontOpZpass);
			glStencilOpSeparate(GL_FRONT, m_state.m_stencilState.m_backOpFail, m_state.m_stencilState.m_backOpZfail, m_state.m_stencilState.m_backOpZpass);
		}
	}
}