#include "ButterflyPrecomputeRenderPass.h"
#include "Level.h"
#include <glm\ext.hpp>

ButterflyPrecomputeRenderPass::ButterflyPrecomputeRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	m_fbo = _fbo;
	m_drawBuffers = { GL_COLOR_ATTACHMENT0 };
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

	m_butterflyPrecomputeShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Ocean/butterflyPrecompute.frag");

	for (int i = 0; i < 512; ++i)
	{
		m_uJ.push_back(m_butterflyPrecomputeShader->createUniform(std::string("uJ") + "[" + std::to_string(i) + "]"));
	}
	m_uSimulationResolution.create(m_butterflyPrecomputeShader);

	m_fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void ButterflyPrecomputeRenderPass::render(const OceanParams & _water, RenderPass ** _previousRenderPass)
{
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	m_fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	std::unique_ptr<uint32_t[]> bitReversedIndices = std::make_unique<uint32_t[]>(static_cast<size_t>(_water.m_simulationResolution));

	for (std::uint32_t i = 0; i < _water.m_simulationResolution; ++i)
	{
		std::uint32_t x = glm::bitfieldReverse(i);
		x = glm::bitfieldRotateRight(x, glm::log2(_water.m_simulationResolution));
		bitReversedIndices[static_cast<size_t>(i)] = x;
	}

	m_butterflyPrecomputeShader->bind();
	m_uSimulationResolution.set(_water.m_simulationResolution);
	for (size_t i = 0; i < static_cast<size_t>(_water.m_simulationResolution); ++i)
	{
		m_butterflyPrecomputeShader->setUniform(m_uJ[i], (int)bitReversedIndices[i]);
	}

	m_fullscreenTriangle->getSubMesh()->render();
}
