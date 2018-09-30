#include "CocNeighborTileMaxRenderPass.h"

CocNeighborTileMaxRenderPass::CocNeighborTileMaxRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
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

	neighborTileMaxShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/DepthOfField/cocNeighborTileMax.frag");

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void CocNeighborTileMaxRenderPass::render(GLuint _cocTileMaxTexture, GLuint _cocNeighborTileMaxTexture, RenderPass ** _previousRenderPass)
{
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	neighborTileMaxShader->bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _cocTileMaxTexture);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _cocNeighborTileMaxTexture, 0);

	fullscreenTriangle->getSubMesh()->render();
}