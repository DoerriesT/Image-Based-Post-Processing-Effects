#include "SSAORenderPass.h"
#include <random>
#include "Graphics\Effects.h"
#include "Graphics\OpenGL\RenderData.h"
#include "Graphics\OpenGL\GLTimerQuery.h"

SSAORenderPass::SSAORenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
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

	m_ssaoShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/SSAO/ssao.frag");

	m_uView.create(m_ssaoShader);
	m_uProjection.create(m_ssaoShader);
	m_uInverseProjection.create(m_ssaoShader);
	for (int i = 0; i < 64; ++i)
	{
		m_uSamples.push_back(m_ssaoShader->createUniform(std::string("uSamples") + "[" + std::to_string(i) + "]"));
	}
	m_uKernelSize.create(m_ssaoShader);
	m_uRadius.create(m_ssaoShader);
	m_uBias.create(m_ssaoShader);
	m_uStrength.create(m_ssaoShader);

	m_fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

double ssaoRenderTime;

void SSAORenderPass::render(const RenderData & _renderData, const Effects & _effects, GLuint _noiseTexture, RenderPass **_previousRenderPass)
{
	GLTimerQuery timer(ssaoRenderTime);
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
	static std::default_random_engine generator;
	static std::vector<glm::vec3> ssaoKernel;
	static bool generateKernel = true;
	static unsigned int currentKernelSize = 16;
	if (currentKernelSize != _effects.m_ssao.m_kernelSize)
	{
		currentKernelSize = _effects.m_ssao.m_kernelSize;
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

	m_fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, _noiseTexture);

	m_ssaoShader->bind();
	m_uView.set(_renderData.m_viewMatrix);
	m_uProjection.set(_renderData.m_projectionMatrix);
	m_uInverseProjection.set(_renderData.m_invProjectionMatrix);
	m_uKernelSize.set((int)currentKernelSize);
	m_uRadius.set(_effects.m_ssao.m_radius);
	m_uBias.set(_effects.m_ssao.m_bias);
	m_uStrength.set(_effects.m_ssao.m_strength);
	if (generateKernel)
	{
		generateKernel = false;
		for (size_t i = 0; i < static_cast<size_t>(currentKernelSize); ++i)
		{
			m_ssaoShader->setUniform(m_uSamples[i], ssaoKernel[i]);
		}
	}

	m_fullscreenTriangle->getSubMesh()->render();
}
