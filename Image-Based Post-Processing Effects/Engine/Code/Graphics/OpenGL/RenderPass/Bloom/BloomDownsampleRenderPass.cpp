#include "BloomDownsampleRenderPass.h"

BloomDownsampleRenderPass::BloomDownsampleRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
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

	downsampleShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Misc/downsample.frag");

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void BloomDownsampleRenderPass::render(GLuint * _textureChain, size_t _textureCount, GLuint *_fbos, size_t _fboCount, RenderPass ** _previousRenderPass)
{
	fbo = _fbos[0];
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	glm::vec2 viewports[] =
	{
		glm::vec2(state.viewportState.width, state.viewportState.height),
		glm::vec2(state.viewportState.width / 2.0f, state.viewportState.height / 2.0f),
		glm::vec2(state.viewportState.width / 4.0f, state.viewportState.height / 4.0f),
		glm::vec2(state.viewportState.width / 8.0f, state.viewportState.height / 8.0f),
		glm::vec2(state.viewportState.width / 16.0f, state.viewportState.height / 16.0f),
		glm::vec2(state.viewportState.width / 32.0f, state.viewportState.height / 32.0f)
	};

	downsampleShader->bind();

	for (size_t i = 0; i < _fboCount; ++i)
	{
		if (i > 0)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, _fbos[i]);
			glViewport(0, 0, (GLsizei)viewports[i].x, (GLsizei)viewports[i].y);
		}
		

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _textureChain[i]);

		fullscreenTriangle->getSubMesh()->render();
	}

	fbo = _fbos[_fboCount - 1];
	glViewport(0, 0, (GLsizei)viewports[0].x, (GLsizei)viewports[0].y);
}