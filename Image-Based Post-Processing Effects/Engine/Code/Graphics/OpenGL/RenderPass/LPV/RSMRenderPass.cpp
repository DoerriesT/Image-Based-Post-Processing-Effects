#include "RSMRenderPass.h"
#include "Graphics\EntityRenderData.h"
#include "Graphics\OpenGL\RenderData.h"
#include "Graphics\Scene.h"
#include "Utilities\ContainerUtility.h"
#include "Graphics\Mesh.h"
#include "EntityComponentSystem\Component.h"
#include <glm\mat4x4.hpp>
#include <glm\vec3.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\ext.hpp>
#include "Graphics\Texture.h"
#include "Level.h"

RSMRenderPass::RSMRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	m_fbo = _fbo;
	m_drawBuffers = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	m_state.m_blendState.m_enabled = false;
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

	m_rsmPassShader = ShaderProgram::createShaderProgram("Resources/Shaders/LPV/rsm.vert", "Resources/Shaders/LPV/rsm.frag");

	m_uModelMatrix.create(m_rsmPassShader);
	m_uModelViewProjectionMatrix.create(m_rsmPassShader);
	m_uAtlasData.create(m_rsmPassShader);
	m_uHasTexture.create(m_rsmPassShader);
	m_uAlbedo.create(m_rsmPassShader);
	m_uLightColor.create(m_rsmPassShader);
	m_uLightDir.create(m_rsmPassShader);
}

void RSMRenderPass::render(const glm::mat4 &_viewProjection, std::shared_ptr<DirectionalLight> _light, const Scene & _scene, RenderPass ** _previousRenderPass)
{
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_rsmPassShader->bind();

	m_uLightDir.set(_light->getDirection());
	m_uLightColor.set(_light->getColor());

	const std::vector<std::unique_ptr<EntityRenderData>> &data = _scene.getData();

	std::shared_ptr<SubMesh> currentMesh = nullptr;
	bool enabledMesh = false;

	for (std::size_t i = 0; i < data.size(); ++i)
	{
		const std::unique_ptr<EntityRenderData> &entityRenderData = data[i];

		// skip this iteration if its supposed to be rendered with another method or does not have sufficient components
		if (!entityRenderData->m_modelComponent || !entityRenderData->m_transformationComponent)
		{
			continue;
		}

		if (currentMesh != entityRenderData->m_mesh)
		{
			currentMesh = entityRenderData->m_mesh;
			enabledMesh = false;
		}

		// skip this mesh if its transparent
		if (entityRenderData->m_transparencyComponent && ContainerUtility::contains(entityRenderData->m_transparencyComponent->m_transparentSubMeshes, currentMesh))
		{
			continue;
		}

		// skip this iteration if the mesh is not yet valid
		if (!currentMesh || !currentMesh->isValid())
		{
			continue;
		}

		glm::mat4 modelMatrix = glm::translate(entityRenderData->m_transformationComponent->m_position)
			* glm::mat4_cast(entityRenderData->m_transformationComponent->m_rotation)
			* glm::scale(glm::vec3(entityRenderData->m_transformationComponent->m_scale));

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

		glm::mat4 mvpTransformation = _viewProjection * modelMatrix;

		m_uModelMatrix.set(modelMatrix);
		m_uAtlasData.set(glm::vec4(1.0f / columns, 1.0f / rows, textureOffset));
		m_uModelViewProjectionMatrix.set(mvpTransformation);

		if (!enabledMesh)
		{
			enabledMesh = true;
			currentMesh->enableVertexAttribArrays();
		}

		if (std::shared_ptr<Texture> albedoMap = entityRenderData->m_material->getAlbedoMap())
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, albedoMap->getId());
			m_uHasTexture.set(true);
		}
		else
		{
			m_uHasTexture.set(false);
			m_uAlbedo.set(entityRenderData->m_material->getAlbedo());
		}

		currentMesh->render();
	}
}
