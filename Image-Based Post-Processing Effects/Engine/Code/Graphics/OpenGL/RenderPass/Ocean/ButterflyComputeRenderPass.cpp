#include "ButterflyComputeRenderPass.h"
#include "Level.h"
#include <glm\ext.hpp>

ButterflyComputeRenderPass::ButterflyComputeRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	fbo = _fbo;
	drawBuffers = { GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7 };
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

	butterflyComputeShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Water/butterflyCompute.frag");

	uSimulationResolutionBC.create(butterflyComputeShader);
	uStageBC.create(butterflyComputeShader);
	uStagesBC.create(butterflyComputeShader);
	uDirectionBC.create(butterflyComputeShader);

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void ButterflyComputeRenderPass::render(const Water &_water, GLuint _twiddleIndicesTexture, GLuint *_readTextures, RenderPass ** _previousRenderPass)
{
	drawBuffers = { GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7 };
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	butterflyComputeShader->bind();
	uSimulationResolutionBC.set(_water.simulationResolution);
	uStagesBC.set(glm::log2(_water.simulationResolution));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _twiddleIndicesTexture);
	glActiveTexture(GL_TEXTURE1);

	GLenum pingPongDrawBuffers[] = { GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7 };
	GLenum sourceDrawBuffers[] = { GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
	GLuint pingPongReadBuffers[] = { _readTextures[3], _readTextures[4], _readTextures[5] };
	GLuint sourceReadBuffers[] = { _readTextures[0], _readTextures[1], _readTextures[2] };

	GLenum *drawBuffersArr[] = { pingPongDrawBuffers, sourceDrawBuffers };
	GLuint *inputTextures[] = { sourceReadBuffers, pingPongReadBuffers };
	unsigned int drawBuffer = 0;

	for (int i = 0; i < 2; ++i)
	{
		uDirectionBC.set(i);

		for (int j = 0; unsigned int(j) < glm::log2(_water.simulationResolution); ++j)
		{
			uStageBC.set(j);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, inputTextures[drawBuffer][0]);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, inputTextures[drawBuffer][1]);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, inputTextures[drawBuffer][2]);
			glDrawBuffers(3, drawBuffersArr[drawBuffer]);

			fullscreenTriangle->getSubMesh()->render();

			drawBuffer = 1 - drawBuffer;
		}
	}

	drawBuffers = { drawBuffersArr[drawBuffer][0], drawBuffersArr[drawBuffer][1], drawBuffersArr[drawBuffer][2] };
}
