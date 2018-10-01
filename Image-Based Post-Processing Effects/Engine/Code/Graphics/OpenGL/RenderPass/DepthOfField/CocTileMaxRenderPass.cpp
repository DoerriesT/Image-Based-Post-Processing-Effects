#include "CocTileMaxRenderPass.h"

CocTileMaxRenderPass::CocTileMaxRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
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

	tileMaxShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/DepthOfField/cocTileMax.frag");

	uDirectionCOCTM.create(tileMaxShader);
	uTileSizeCOCTM.create(tileMaxShader);

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void CocTileMaxRenderPass::render(GLuint _inputCocTexture, GLuint _intermediaryTexture, GLuint _cocTileMaxTexture, unsigned int _tileSize, RenderPass ** _previousRenderPass)
{
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	tileMaxShader->bind();

	uTileSizeCOCTM.set(_tileSize);

	glActiveTexture(GL_TEXTURE0);

	// fullscreen to first step
	{
		glViewport(0, 0, width / _tileSize, height);
		glBindTexture(GL_TEXTURE_2D, _inputCocTexture);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _intermediaryTexture, 0);

		uDirectionCOCTM.set(false);

		fullscreenTriangle->getSubMesh()->render();
	}

	// first to second step
	{
		glViewport(0, 0, width / _tileSize, height / _tileSize);
		glBindTexture(GL_TEXTURE_2D, _intermediaryTexture);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _cocTileMaxTexture, 0);

		uDirectionCOCTM.set(true);

		fullscreenTriangle->getSubMesh()->render();
	}

	glViewport(0, 0, state.viewportState.width, state.viewportState.height);
}

void CocTileMaxRenderPass::resize(unsigned int _width, unsigned int _height)
{
	RenderPass::resize(_width, _height);
	width = _width;
	height = _height;
}