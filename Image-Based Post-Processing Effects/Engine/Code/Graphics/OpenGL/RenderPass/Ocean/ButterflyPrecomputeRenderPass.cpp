#include "ButterflyPrecomputeRenderPass.h"
#include "Level.h"
#include <glm\ext.hpp>

ButterflyPrecomputeRenderPass::ButterflyPrecomputeRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
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

	butterflyPrecomputeShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Ocean/butterflyPrecompute.frag");

	for (int i = 0; i < 512; ++i)
	{
		uJBP.push_back(butterflyPrecomputeShader->createUniform(std::string("uJ") + "[" + std::to_string(i) + "]"));
	}
	uSimulationResolutionBP.create(butterflyPrecomputeShader);

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void ButterflyPrecomputeRenderPass::render(const Water & _water, RenderPass ** _previousRenderPass)
{
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	std::unique_ptr<uint32_t[]> bitReversedIndices = std::make_unique<uint32_t[]>(static_cast<size_t>(_water.simulationResolution));

	for (std::uint32_t i = 0; i < _water.simulationResolution; ++i)
	{
		std::uint32_t x = glm::bitfieldReverse(i);
		x = glm::bitfieldRotateRight(x, glm::log2(_water.simulationResolution));
		bitReversedIndices[static_cast<size_t>(i)] = x;
	}

	butterflyPrecomputeShader->bind();
	uSimulationResolutionBP.set(_water.simulationResolution);
	for (size_t i = 0; i < static_cast<size_t>(_water.simulationResolution); ++i)
	{
		butterflyPrecomputeShader->setUniform(uJBP[i], (int)bitReversedIndices[i]);
	}

	fullscreenTriangle->getSubMesh()->render();
}
