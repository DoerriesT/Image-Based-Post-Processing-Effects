#include "DeferredEnvironmentProbeRenderPass.h"
#include <glm\mat4x4.hpp>
#include <glm\vec3.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\ext.hpp>
#include "Level.h"
#include "Graphics\OpenGL\RenderData.h"
#include "Graphics\Texture.h"
#include "Graphics\Effects.h"

static const char *SSAO_ENABLED = "SSAO_ENABLED";

DeferredEnvironmentProbeRenderPass::DeferredEnvironmentProbeRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	fbo = _fbo;
	drawBuffers = { GL_COLOR_ATTACHMENT4 };
	state.blendState.enabled = true;
	state.blendState.sFactor = GL_ONE;
	state.blendState.dFactor = GL_ONE;
	state.cullFaceState.enabled = true;
	state.cullFaceState.face = GL_FRONT;
	state.depthState.enabled = false;
	state.depthState.func = GL_LEQUAL;
	state.depthState.mask = GL_FALSE;
	state.stencilState.enabled = true;
	state.stencilState.frontFunc = state.stencilState.backFunc = GL_NOTEQUAL;
	state.stencilState.frontRef = state.stencilState.backRef = 0;
	state.stencilState.frontMask = state.stencilState.backMask = 0xFF;
	state.stencilState.frontOpFail = state.stencilState.backOpFail = GL_KEEP;
	state.stencilState.frontOpZfail = state.stencilState.backOpZfail = GL_KEEP;
	state.stencilState.frontOpZpass = state.stencilState.backOpZpass = GL_KEEP;

	resize(_width, _height);

	deferredEnvironmentProbePassShader = ShaderProgram::createShaderProgram(
		{
		{ ShaderProgram::ShaderType::FRAGMENT, SSAO_ENABLED, 0 },
		},
		"Resources/Shaders/Lighting/deferredEnvironmentProbe.vert", 
		"Resources/Shaders/Lighting/deferredEnvironmentProbe.frag");

	uModelViewProjection.create(deferredEnvironmentProbePassShader);
	uInverseView.create(deferredEnvironmentProbePassShader);
	uInverseProjection.create(deferredEnvironmentProbePassShader);
	uBoxMin.create(deferredEnvironmentProbePassShader);
	uBoxMax.create(deferredEnvironmentProbePassShader);
	uProbePosition.create(deferredEnvironmentProbePassShader);

	boxMesh = Mesh::createMesh("Resources/Models/cube.mesh", 1, true);
}

void DeferredEnvironmentProbeRenderPass::render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const Effects &_effects, GLuint _ssaoTexture, GLuint _brdfLUT, RenderPass **_previousRenderPass)
{
	if (_level->environment.environmentProbes.empty())
	{
		return;
	}

	drawBuffers[0] = _renderData.frame % 2 ? GL_COLOR_ATTACHMENT5 : GL_COLOR_ATTACHMENT4;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	// shader permutations
	{
		const auto curDefines = deferredEnvironmentProbePassShader->getDefines();

		bool ssaoEnabled = false;

		for (const auto &define : curDefines)
		{
			if (std::get<0>(define) == ShaderProgram::ShaderType::FRAGMENT)
			{
				if (std::get<1>(define) == SSAO_ENABLED && std::get<2>(define))
				{
					ssaoEnabled = true;
				}
			}
		}

		if (ssaoEnabled != (_effects.ambientOcclusion != AmbientOcclusion::OFF))
		{
			deferredEnvironmentProbePassShader->setDefines(
				{
				{ ShaderProgram::ShaderType::FRAGMENT, SSAO_ENABLED, (_effects.ambientOcclusion != AmbientOcclusion::OFF) },
				}
			);
			uModelViewProjection.create(deferredEnvironmentProbePassShader);
			uInverseView.create(deferredEnvironmentProbePassShader);
			uInverseProjection.create(deferredEnvironmentProbePassShader);
			uBoxMin.create(deferredEnvironmentProbePassShader);
			uBoxMax.create(deferredEnvironmentProbePassShader);
			uProbePosition.create(deferredEnvironmentProbePassShader);
		}
	}

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, _ssaoTexture);
	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, _brdfLUT);

	boxMesh->getSubMesh()->enableVertexAttribArraysPositionOnly();

	deferredEnvironmentProbePassShader->bind();

	uInverseView.set(_renderData.invViewMatrix);
	uInverseProjection.set(_renderData.invProjectionMatrix);

	for (auto probe : _level->environment.environmentProbes)
	{
		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_2D, probe->getReflectionTexture()->getId());

		AxisAlignedBoundingBox aabb = probe->getAxisAlignedBoundingBox();
		glm::vec3 boundingBoxCenter = (aabb.max + aabb.min) * 0.5f;
		glm::vec3 correctedMax = aabb.max - boundingBoxCenter;
		glm::vec3 correctedMin = aabb.min - boundingBoxCenter;
		glm::vec3 boxScale = correctedMax / 0.5f;

		glm::mat4 modelMatrix = glm::translate(boundingBoxCenter)
			* glm::scale(glm::vec3(boxScale));

		uBoxMin.set(aabb.min);
		uBoxMax.set(aabb.max);
		uProbePosition.set(probe->getPosition());
		uModelViewProjection.set(_renderData.viewProjectionMatrix * modelMatrix);

		boxMesh->getSubMesh()->render();
	}
}
