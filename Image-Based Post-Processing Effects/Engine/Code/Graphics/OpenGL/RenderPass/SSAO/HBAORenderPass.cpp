#include "HBAORenderPass.h"
#include <glm\ext.hpp>
#include "Graphics\OpenGL\RenderData.h"
#include "Graphics\Effects.h"

HBAORenderPass::HBAORenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
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

	hbaoShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/SSAO/hbao.frag");

	uFocalLengthHBAO.create(hbaoShader);
	uInverseProjectionHBAO.create(hbaoShader);
	uAOResHBAO.create(hbaoShader);
	uInvAOResHBAO.create(hbaoShader);
	uNoiseScaleHBAO.create(hbaoShader);
	uStrengthHBAO.create(hbaoShader);
	uRadiusHBAO.create(hbaoShader);
	uRadius2HBAO.create(hbaoShader);
	uNegInvR2HBAO.create(hbaoShader);
	uTanBiasHBAO.create(hbaoShader);
	uMaxRadiusPixelsHBAO.create(hbaoShader);
	uNumDirectionsHBAO.create(hbaoShader);
	uNumStepsHBAO.create(hbaoShader);

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void HBAORenderPass::render(const RenderData & _renderData, const Effects & _effects, const GBuffer & _gbuffer, GLuint _noiseTexture, RenderPass **_previousRenderPass)
{
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, _noiseTexture);

	float aspectRatio = _renderData.resolution.second / (float)_renderData.resolution.first;
	float fovy = 2.0f * glm::atan(glm::tan(glm::radians(_renderData.fov) * 0.5f) * aspectRatio);
	glm::vec2 focalLength;
	focalLength.x = 1.0f / tanf(fovy * 0.5f) * aspectRatio;
	focalLength.y = 1.0f / tanf(fovy * 0.5f);

	glm::vec2 res(_renderData.resolution.first, _renderData.resolution.second);
	float radius = 0.3f;

	hbaoShader->bind();
	uFocalLengthHBAO.set(focalLength);
	uInverseProjectionHBAO.set(_renderData.invProjectionMatrix);
	uAOResHBAO.set(res);
	uInvAOResHBAO.set(1.0f / res);
	uNoiseScaleHBAO.set(res * 0.25f);
	uStrengthHBAO.set(_effects.hbao.strength);
	uRadiusHBAO.set(_effects.hbao.radius);
	uRadius2HBAO.set(_effects.hbao.radius * _effects.hbao.radius);
	uNegInvR2HBAO.set(-1.0f / (_effects.hbao.radius * _effects.hbao.radius));
	uTanBiasHBAO.set(_effects.hbao.angleBias);
	uMaxRadiusPixelsHBAO.set(_effects.hbao.maxRadiusPixels);
	uNumDirectionsHBAO.set((float)_effects.hbao.directions);
	uNumStepsHBAO.set((float)_effects.hbao.steps);

	fullscreenTriangle->getSubMesh()->render();
}
