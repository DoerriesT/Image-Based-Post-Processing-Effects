#include "ForwardCustomRenderPass.h"
#include "Graphics\Scene.h"
#include "Graphics\EntityRenderData.h"
#include "Graphics\Mesh.h"
#include "Graphics\OpenGL\RenderData.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "EntityComponentSystem\Component.h"

ForwardCustomRenderPass::ForwardCustomRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
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
}

void ForwardCustomRenderPass::render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const Scene &_scene, RenderPass **_previousRenderPass)
{
	if (!_scene.getCustomTransparencyCount())
	{
		return;
	}

	drawBuffers[0] = _renderData.frame % 2 ? GL_COLOR_ATTACHMENT5 : GL_COLOR_ATTACHMENT4;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	const std::vector<std::unique_ptr<EntityRenderData>> &data = _scene.getData();

	std::shared_ptr<SubMesh> currentMesh = nullptr;
	bool enabledMesh = false;

	for (std::size_t i = 0; i < data.size(); ++i)
	{
		const std::unique_ptr<EntityRenderData> &entityRenderData = data[i];

		// continue if this is a bake and the entity is not static
		if (entityRenderData->transformationComponent && entityRenderData->transformationComponent->mobility != Mobility::STATIC && _renderData.bake)
		{
			continue;
		}

		// skip this iteration if its supposed to be rendered with another method or does not have sufficient components
		if (!entityRenderData->customOpaqueShaderComponent && !entityRenderData->customTransparencyShaderComponent)
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
			currentMesh->enableVertexAttribArrays();
		}

		// we're good to go: render this mesh-entity instance
		entityRenderData->customTransparencyShaderComponent->transparencyShader->bind();
		entityRenderData->customTransparencyShaderComponent->renderTransparency(_renderData, _level, entityRenderData);
	}
}
