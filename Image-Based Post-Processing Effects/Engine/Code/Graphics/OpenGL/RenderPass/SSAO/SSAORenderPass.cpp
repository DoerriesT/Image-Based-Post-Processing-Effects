#include "SSAORenderPass.h"
#include <random>
#include "Graphics\Effects.h"
#include "Graphics\OpenGL\RenderData.h"

SSAORenderPass::SSAORenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
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

	ssaoShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/SSAO/ssao.frag");

	uViewAO.create(ssaoShader);
	uProjectionAO.create(ssaoShader);
	uInverseProjectionAO.create(ssaoShader);
	for (int i = 0; i < 64; ++i)
	{
		uSamplesAO.push_back(ssaoShader->createUniform(std::string("uSamples") + "[" + std::to_string(i) + "]"));
	}
	uKernelSizeAO.create(ssaoShader);
	uRadiusAO.create(ssaoShader);
	uBiasAO.create(ssaoShader);
	uStrengthAO.create(ssaoShader);

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void SSAORenderPass::render(const RenderData & _renderData, const Effects & _effects, const GBuffer &_gbuffer, GLuint _noiseTexture, RenderPass **_previousRenderPass)
{
	drawBuffers[0] = _renderData.frame % 2 ? GL_COLOR_ATTACHMENT2 : GL_COLOR_ATTACHMENT0;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
	static std::default_random_engine generator;
	static std::vector<glm::vec3> ssaoKernel;
	static bool generateKernel = true;
	static unsigned int currentKernelSize = 16;
	if (currentKernelSize != _effects.ssao.kernelSize)
	{
		currentKernelSize = _effects.ssao.kernelSize;
		generateKernel = true;
	}
	if (generateKernel)
	{
		ssaoKernel.clear();
		for (unsigned int i = 0; i < currentKernelSize; ++i)
		{
			glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
			sample = glm::normalize(sample);
			sample *= randomFloats(generator);
			float scale = float(i) / currentKernelSize;

			// scale samples s.t. they're more aligned to center of kernel
			scale = glm::mix(0.1f, 1.0f, scale * scale);
			sample *= scale;
			ssaoKernel.push_back(sample);
		}
	}

	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, _noiseTexture);

	ssaoShader->bind();
	uViewAO.set(_renderData.viewMatrix);
	uProjectionAO.set(_renderData.projectionMatrix);
	uInverseProjectionAO.set(_renderData.invProjectionMatrix);
	uKernelSizeAO.set((int)currentKernelSize);
	uRadiusAO.set(_effects.ssao.radius);
	uBiasAO.set(_effects.ssao.bias);
	uStrengthAO.set(_effects.ssao.strength);
	if (generateKernel)
	{
		generateKernel = false;
		for (unsigned int i = 0; i < currentKernelSize; ++i)
		{
			ssaoShader->setUniform(uSamplesAO[i], ssaoKernel[i]);
		}
	}

	fullscreenTriangle->getSubMesh()->render();
}
