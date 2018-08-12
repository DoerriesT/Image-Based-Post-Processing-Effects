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
	fbo = _fbo;
	drawBuffers = { GL_COLOR_ATTACHMENT0 , GL_COLOR_ATTACHMENT1 , GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
	state.blendState.enabled = false;
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

	gBufferPassShader = ShaderProgram::createShaderProgram("Resources/Shaders/Renderer/gBufferPass.vert", "Resources/Shaders/Renderer/gBufferPass.frag");

	uMaterialG.create(gBufferPassShader);
	uModelViewProjectionMatrixG.create(gBufferPassShader);
	uPrevTransformG.create(gBufferPassShader);
	uAtlasDataG.create(gBufferPassShader);
	uVelG.create(gBufferPassShader);
	uExposureTimeG.create(gBufferPassShader);
	uMaxVelocityMagG.create(gBufferPassShader);
	uCurrTransformG.create(gBufferPassShader);
	uViewMatrixG.create(gBufferPassShader);
	uModelMatrixG.create(gBufferPassShader);
	uCamPosG.create(gBufferPassShader);
}

void GBufferRenderPass::render(const RenderData &_renderData, const Scene &_scene, RenderPass **_previousRenderPass)
{
	drawBuffers[4] = _renderData.frame % 2 ? GL_COLOR_ATTACHMENT5 : GL_COLOR_ATTACHMENT4;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	gBufferPassShader->bind();

	const std::vector<std::unique_ptr<EntityRenderData>> &data = _scene.getData();

	std::shared_ptr<SubMesh> currentMesh = nullptr;
	bool enabledMesh = false;

	for (std::size_t i = 0; i < data.size(); ++i)
	{
		const std::unique_ptr<EntityRenderData> &entityRenderData = data[i];

		// skip this iteration if its supposed to be rendered with another method or does not have sufficient components
		if (entityRenderData->customOpaqueShaderComponent ||
			entityRenderData->customTransparencyShaderComponent ||
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

		// skip this mesh if its transparent
		if (entityRenderData->transparencyComponent && ContainerUtility::contains(entityRenderData->transparencyComponent->transparentSubMeshes, currentMesh))
		{
			continue;
		}

		// skip this iteration if the mesh is not yet valid
		if (!currentMesh || !currentMesh->isValid())
		{
			continue;
		}

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


		uCamPosG.set(_renderData.cameraPosition);
		uViewMatrixG.set(glm::mat3(_renderData.viewMatrix));
		uModelMatrixG.set(modelMatrix);
		uAtlasDataG.set(glm::vec4(1.0f / columns, 1.0f / rows, textureOffset));
		uModelViewProjectionMatrixG.set(mvpTransformation);
		uPrevTransformG.set(prevTransformation);
		uCurrTransformG.set(_renderData.invJitter * mvpTransformation);
		uVelG.set(entityRenderData->transformationComponent->vel / glm::vec2(_renderData.resolution.first, _renderData.resolution.second));
		const float frameRateTarget = 60.0f;
		uExposureTimeG.set((float(Engine::getFps()) / frameRateTarget));
		const float tileSize = 40.0f;
		uMaxVelocityMagG.set(glm::length(glm::vec2(1.0f) / glm::vec2(_renderData.resolution.first, _renderData.resolution.second)) * tileSize);

		entityRenderData->transformationComponent->prevTransformation = modelMatrix;

		if (entityRenderData->outlineComponent)
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
		uMaterialG.set(entityRenderData->material);
		entityRenderData->material->bindTextures();

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