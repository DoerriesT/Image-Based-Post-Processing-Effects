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
	fbo = _fbo;
	drawBuffers = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	state.blendState.enabled = false;
	state.cullFaceState.enabled = true;
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

	rsmPassShader = ShaderProgram::createShaderProgram("Resources/Shaders/LPV/rsm.vert", "Resources/Shaders/LPV/rsm.frag");

	uModelMatrix.create(rsmPassShader);
	uModelViewProjectionMatrix.create(rsmPassShader);
	uAtlasData.create(rsmPassShader);
	uHasTexture.create(rsmPassShader);
	uAlbedo.create(rsmPassShader);
	uLightColor.create(rsmPassShader);
	uLightDir.create(rsmPassShader);
}

void RSMRenderPass::render(const glm::mat4 &_viewProjection, std::shared_ptr<DirectionalLight> _light, const Scene & _scene, RenderPass ** _previousRenderPass)
{
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	rsmPassShader->bind();

	uLightDir.set(_light->getDirection());
	uLightColor.set(_light->getColor());

	const std::vector<std::unique_ptr<EntityRenderData>> &data = _scene.getData();

	std::shared_ptr<SubMesh> currentMesh = nullptr;
	bool enabledMesh = false;

	for (std::size_t i = 0; i < data.size(); ++i)
	{
		const std::unique_ptr<EntityRenderData> &entityRenderData = data[i];

		// skip this iteration if its supposed to be rendered with another method or does not have sufficient components
		if (!entityRenderData->modelComponent || !entityRenderData->transformationComponent)
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

		glm::mat4 mvpTransformation = _viewProjection * modelMatrix;

		uModelMatrix.set(modelMatrix);
		uAtlasData.set(glm::vec4(1.0f / columns, 1.0f / rows, textureOffset));
		uModelViewProjectionMatrix.set(mvpTransformation);

		if (!enabledMesh)
		{
			enabledMesh = true;
			currentMesh->enableVertexAttribArrays();
		}

		if (std::shared_ptr<Texture> albedoMap = entityRenderData->material->getAlbedoMap())
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, albedoMap->getId());
			uHasTexture.set(true);
		}
		else
		{
			uHasTexture.set(false);
			uAlbedo.set(entityRenderData->material->getAlbedo());
		}

		currentMesh->render();
	}
}
