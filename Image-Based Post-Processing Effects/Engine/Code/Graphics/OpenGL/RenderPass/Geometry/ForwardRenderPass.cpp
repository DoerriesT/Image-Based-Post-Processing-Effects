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
	fbo = _fbo;
	drawBuffers = { GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT3 };
	state.blendState.enabled = true;
	state.blendState.sFactor = GL_SRC_ALPHA;
	state.blendState.dFactor = GL_ONE_MINUS_SRC_ALPHA;
	state.cullFaceState.enabled = true;
	state.cullFaceState.face = GL_BACK;
	state.depthState.enabled = true;
	state.depthState.func = GL_LEQUAL;
	state.depthState.mask = GL_TRUE;
	state.stencilState.enabled = true;
	state.stencilState.frontFunc = state.stencilState.backFunc = GL_ALWAYS;
	state.stencilState.frontRef = state.stencilState.backRef = 1;
	state.stencilState.frontMask = state.stencilState.backMask = 0xFF;
	state.stencilState.frontOpFail = state.stencilState.backOpFail = GL_KEEP;
	state.stencilState.frontOpZfail = state.stencilState.backOpZfail = GL_KEEP;
	state.stencilState.frontOpZpass = state.stencilState.backOpZpass = GL_KEEP;

	resize(_width, _height);

	transparencyShader = ShaderProgram::createShaderProgram("Resources/Shaders/Renderer/transparencyForward.vert", "Resources/Shaders/Renderer/transparencyForward.frag");

	uViewMatrixT.create(transparencyShader);
	uPrevTransformT.create(transparencyShader);
	uModelViewProjectionMatrixT.create(transparencyShader);
	uModelMatrixT.create(transparencyShader);
	uAtlasDataT.create(transparencyShader);
	uMaterialT.create(transparencyShader);
	uDirectionalLightT.create(transparencyShader);
	uRenderDirectionalLightT.create(transparencyShader);
	uCamPosT.create(transparencyShader);
	uShadowsEnabledT.create(transparencyShader);
	uCurrTransformT.create(transparencyShader);
}

void ForwardRenderPass::render(const RenderData & _renderData, const std::shared_ptr<Level>& _level, const Scene & _scene, RenderPass **_previousRenderPass)
{
	if (!_scene.getTransparencyCount())
	{
		return;
	}

	drawBuffers[0] = _renderData.frame % 2 ? GL_COLOR_ATTACHMENT5 : GL_COLOR_ATTACHMENT4;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	transparencyShader->bind();

	if (!_level->lights.directionalLights.empty())
	{
		if (_level->lights.directionalLights[0]->isRenderShadows())
		{
			glActiveTexture(GL_TEXTURE15);
			glBindTexture(GL_TEXTURE_2D_ARRAY, _level->lights.directionalLights[0]->getShadowMap());
		}
		uRenderDirectionalLightT.set(true);
		uDirectionalLightT.set(_level->lights.directionalLights[0]);
	}
	else
	{
		uRenderDirectionalLightT.set(false);
	}
	uShadowsEnabledT.set(_renderData.shadows);
	uCamPosT.set(_renderData.cameraPosition);
	uViewMatrixT.set(_renderData.viewMatrix);

	const std::vector<std::unique_ptr<EntityRenderData>> &data = _scene.getData();

	std::shared_ptr<SubMesh> currentMesh = nullptr;
	bool enabledMesh = false;

	for (std::size_t i = 0; i < data.size(); ++i)
	{
		const std::unique_ptr<EntityRenderData> &entityRenderData = data[i];

		// skip this iteration if its supposed to be rendered with another method or does not have sufficient components
		if (entityRenderData->customOpaqueShaderComponent ||
			entityRenderData->customTransparencyShaderComponent ||
			!entityRenderData->transparencyComponent ||
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

		// skip this mesh if its not transparent
		if (entityRenderData->transparencyComponent && !ContainerUtility::contains(entityRenderData->transparencyComponent->transparentSubMeshes, currentMesh))
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
		uMaterialT.set(entityRenderData->material);
		entityRenderData->material->bindTextures();

		glm::mat4 modelMatrix = glm::translate(entityRenderData->transformationComponent->position)
			* glm::mat4_cast(entityRenderData->transformationComponent->rotation)
			* glm::scale(glm::vec3(entityRenderData->transformationComponent->scale));

		int rows = 1;
		int columns = 1;
		glm::vec2 textureOffset;
		TextureAtlasIndexComponent *textureAtlasComponent = entityRenderData->textureAtlasIndexComponent;
		if (textureAtlasComponent && ContainerUtility::contains(textureAtlasComponent->meshToIndexMap, currentMesh))
		{
			rows = textureAtlasComponent->rows;
			columns = textureAtlasComponent->columns;
			int texPos = textureAtlasComponent->meshToIndexMap[currentMesh];
			int col = texPos % columns;
			int row = texPos / columns;
			textureOffset = glm::vec2((float)col / columns, (float)row / rows);
		}

		glm::mat4 mvpTransformation = _renderData.viewProjectionMatrix * modelMatrix;
		const float cameraMovementStrength = 1.0f;
		glm::mat4 prevTransformation = glm::mix(_renderData.invJitter * _renderData.viewProjectionMatrix, _renderData.prevInvJitter * _renderData.prevViewProjectionMatrix, cameraMovementStrength) * entityRenderData->transformationComponent->prevTransformation;

		uAtlasDataT.set(glm::vec4(1.0f / columns, 1.0f / rows, textureOffset));
		uModelMatrixT.set(modelMatrix);
		uModelViewProjectionMatrixT.set(mvpTransformation);
		uPrevTransformT.set(prevTransformation);
		uCurrTransformT.set(_renderData.invJitter * mvpTransformation);

		entityRenderData->transformationComponent->prevTransformation = modelMatrix;

		if (entityRenderData->outlineComponent)
		{
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
			glStencilMask(0xFF);
		}

		currentMesh->render();

		if (entityRenderData->outlineComponent)
		{
			glStencilMaskSeparate(GL_FRONT, state.stencilState.frontMask);
			glStencilMaskSeparate(GL_BACK, state.stencilState.backMask);
			glStencilOpSeparate(GL_FRONT, state.stencilState.frontOpFail, state.stencilState.frontOpZfail, state.stencilState.frontOpZpass);
			glStencilOpSeparate(GL_FRONT, state.stencilState.backOpFail, state.stencilState.backOpZfail, state.stencilState.backOpZpass);
		}
	}
}