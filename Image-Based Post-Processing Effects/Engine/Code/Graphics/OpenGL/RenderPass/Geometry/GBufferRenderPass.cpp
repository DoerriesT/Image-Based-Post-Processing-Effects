#include "GBufferRenderPass.h"
#include <glm\mat4x4.hpp>
#include <glm\vec3.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\ext.hpp>
#include "Graphics\OpenGL\RenderData.h"
#include "Graphics\Scene.h"
#include "Graphics\EntityRenderData.h"
#include "Utilities\ContainerUtility.h"
#include "EntityComponentSystem\Component.h"
#include "Graphics\Mesh.h"
#include "Engine.h"


GBufferRenderPass::GBufferRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	m_fbo = _fbo;
	m_drawBuffers = { GL_COLOR_ATTACHMENT0 , GL_COLOR_ATTACHMENT1 , GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
	m_state.m_blendState.m_enabled = false;
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

	m_gBufferPassShader = ShaderProgram::createShaderProgram("Resources/Shaders/Geometry/geometry.vert", "Resources/Shaders/Geometry/geometry.frag");

	m_uMaterial.create(m_gBufferPassShader);
	m_uModelViewProjectionMatrix.create(m_gBufferPassShader);
	m_uPrevTransform.create(m_gBufferPassShader);
	m_uAtlasData.create(m_gBufferPassShader);
	m_uVel.create(m_gBufferPassShader);
	m_uExposureTime.create(m_gBufferPassShader);
	m_uMaxVelocityMag.create(m_gBufferPassShader);
	m_uCurrTransform.create(m_gBufferPassShader);
	m_uViewMatrix.create(m_gBufferPassShader);
	m_uModelMatrix.create(m_gBufferPassShader);
	m_uCamPos.create(m_gBufferPassShader);
}

void GBufferRenderPass::render(const RenderData &_renderData, const Scene &_scene, RenderPass **_previousRenderPass)
{
	m_drawBuffers[4] = _renderData.m_frame % 2 ? GL_COLOR_ATTACHMENT5 : GL_COLOR_ATTACHMENT4;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	m_gBufferPassShader->bind();

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

		glm::mat4 transformation = entityRenderData->m_transformationComponent->m_transformation;
		glm::mat4 modelViewProjection = _renderData.m_viewProjectionMatrix * transformation;
		glm::mat4 prevModelViewProjection = _renderData.m_prevInvJitter * _renderData.m_prevViewProjectionMatrix * entityRenderData->m_transformationComponent->m_prevTransformation;


		m_uCamPos.set(_renderData.m_cameraPosition);
		m_uViewMatrix.set(glm::mat3(_renderData.m_viewMatrix));
		m_uModelMatrix.set(transformation);
		m_uAtlasData.set(glm::vec4(1.0f / columns, 1.0f / rows, textureOffset));
		m_uModelViewProjectionMatrix.set(modelViewProjection);
		m_uPrevTransform.set(prevModelViewProjection);
		m_uCurrTransform.set(_renderData.m_invJitter * modelViewProjection);
		m_uVel.set(entityRenderData->m_transformationComponent->m_vel / glm::vec2(_renderData.m_resolution.first, _renderData.m_resolution.second));
		const float frameRateTarget = 60.0f;
		m_uExposureTime.set((float(Engine::getFps()) / frameRateTarget));
		const float tileSize = 40.0f;
		m_uMaxVelocityMag.set(glm::length(glm::vec2(1.0f) / glm::vec2(_renderData.m_resolution.first, _renderData.m_resolution.second)) * tileSize);

		if (entityRenderData->m_outlineComponent)
		{
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
			glStencilMask(0xFF);
		}

		if (!enabledMesh)
		{
			enabledMesh = true;
			currentMesh->enableVertexAttribArrays();
		}

		// we're good to go: render this mesh-entity instance
		m_uMaterial.set(entityRenderData->m_material);
		entityRenderData->m_material->bindTextures();

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