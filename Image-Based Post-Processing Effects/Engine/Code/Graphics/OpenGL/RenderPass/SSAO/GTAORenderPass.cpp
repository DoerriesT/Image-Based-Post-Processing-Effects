#include "GTAORenderPass.h"
#include <glm\ext.hpp>
#include "Graphics\OpenGL\RenderData.h"
#include "Graphics\Effects.h"

GTAORenderPass::GTAORenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	fbo = _fbo;
	drawBuffers = { GL_COLOR_ATTACHMENT0 };
	state.blendState.enabled = false;
	state.blendState.sFactor = GL_ONE;
	state.blendState.dFactor = GL_ONE;
	state.cullFaceState.enabled = false;
	state.cullFaceState.face = GL_BACK;
	state.depthState.enabled = false;
	state.depthState.func = GL_LEQUAL;
	state.depthState.mask = GL_FALSE;
	state.stencilState.enabled = false;
	state.stencilState.frontFunc = state.stencilState.backFunc = GL_ALWAYS;
	state.stencilState.frontRef = state.stencilState.backRef = 1;
	state.stencilState.frontMask = state.stencilState.backMask = 0xFF;
	state.stencilState.frontOpFail = state.stencilState.backOpFail = GL_KEEP;
	state.stencilState.frontOpZfail = state.stencilState.backOpZfail = GL_KEEP;
	state.stencilState.frontOpZpass = state.stencilState.backOpZpass = GL_KEEP;

	resize(_width, _height);

	gtaoShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/SSAO/gtao.frag");

	uFocalLengthGTAO.create(gtaoShader);
	uInverseProjectionGTAO.create(gtaoShader);
	uAOResGTAO.create(gtaoShader);
	uInvAOResGTAO.create(gtaoShader);
	uStrengthGTAO.create(gtaoShader);
	uRadiusGTAO.create(gtaoShader);
	uMaxRadiusPixelsGTAO.create(gtaoShader);
	uNumStepsGTAO.create(gtaoShader);
	uFrameGTAO.create(gtaoShader);

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void GTAORenderPass::render(const RenderData &_renderData, const Effects &_effects, const GBuffer &_gbuffer, RenderPass **_previousRenderPass)
{
	drawBuffers[0] = _renderData.frame % 2 ? GL_COLOR_ATTACHMENT2 : GL_COLOR_ATTACHMENT0;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	float aspectRatio = _renderData.resolution.second / (float)_renderData.resolution.first;
	float fovy = 2.0f * glm::atan(glm::tan(glm::radians(_renderData.fov) * 0.5f) * aspectRatio);
	float focalLength;
	focalLength = 1.0f / tanf(fovy * 0.5f) * aspectRatio;

	glm::vec2 res(_renderData.resolution.first, _renderData.resolution.second);

	gtaoShader->bind();
	uFrameGTAO.set(_renderData.frame % 12);
	uFocalLengthGTAO.set(focalLength);
	uInverseProjectionGTAO.set(_renderData.invProjectionMatrix);
	uAOResGTAO.set(res);
	uInvAOResGTAO.set(1.0f / res);
	uStrengthGTAO.set(_effects.gtao.strength);
	uRadiusGTAO.set(_effects.gtao.radius);
	uMaxRadiusPixelsGTAO.set(_effects.gtao.maxRadiusPixels);
	uNumStepsGTAO.set((float)_effects.gtao.steps);

	fullscreenTriangle->getSubMesh()->render();
}
