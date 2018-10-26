#include "ButterflyComputeRenderPass.h"
#include "Level.h"
#include <glm\ext.hpp>

ButterflyComputeRenderPass::ButterflyComputeRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	m_fbo = _fbo;
	m_drawBuffers = { GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7 };
	m_state.m_blendState.m_enabled = false;
	m_state.m_blendState.m_sFactor = GL_ONE;
	m_state.m_blendState.m_dFactor = GL_ONE;
	m_state.m_cullFaceState.m_enabled = false;
	m_state.m_cullFaceState.m_face = GL_BACK;
	m_state.m_depthState.m_enabled = false;
	m_state.m_depthState.m_func = GL_LEQUAL;
	m_state.m_depthState.m_mask = GL_FALSE;
	m_state.m_stencilState.m_enabled = false;
	m_state.m_stencilState.m_frontFunc = m_state.m_stencilState.m_backFunc = GL_ALWAYS;
	m_state.m_stencilState.m_frontRef = m_state.m_stencilState.m_backRef = 1;
	m_state.m_stencilState.m_frontMask = m_state.m_stencilState.m_backMask = 0xFF;
	m_state.m_stencilState.m_frontOpFail = m_state.m_stencilState.m_backOpFail = GL_KEEP;
	m_state.m_stencilState.m_frontOpZfail = m_state.m_stencilState.m_backOpZfail = GL_KEEP;
	m_state.m_stencilState.m_frontOpZpass = m_state.m_stencilState.m_backOpZpass = GL_KEEP;

	resize(_width, _height);

	m_butterflyComputeShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Ocean/butterflyCompute.frag");

	m_uSimulationResolution.create(m_butterflyComputeShader);
	m_uStage.create(m_butterflyComputeShader);
	m_uStages.create(m_butterflyComputeShader);
	m_uDirection.create(m_butterflyComputeShader);

	m_fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void ButterflyComputeRenderPass::render(const OceanParams &_water, GLuint _twiddleIndicesTexture, GLuint *_readTextures, RenderPass ** _previousRenderPass)
{
	m_drawBuffers = { GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7 };
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	m_fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	m_butterflyComputeShader->bind();
	m_uSimulationResolution.set(_water.m_simulationResolution);
	m_uStages.set(glm::log2(_water.m_simulationResolution));

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
		m_uDirection.set(i);

		for (int j = 0; unsigned int(j) < glm::log2(_water.m_simulationResolution); ++j)
		{
			m_uStage.set(j);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, inputTextures[drawBuffer][0]);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, inputTextures[drawBuffer][1]);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, inputTextures[drawBuffer][2]);
			glDrawBuffers(3, drawBuffersArr[drawBuffer]);

			m_fullscreenTriangle->getSubMesh()->render();

			drawBuffer = 1 - drawBuffer;
		}
	}

	m_drawBuffers = { drawBuffersArr[drawBuffer][0], drawBuffersArr[drawBuffer][1], drawBuffersArr[drawBuffer][2] };
}
