#include "SMAAEdgeDetectionRenderPass.h"

SMAAEdgeDetectionRenderPass::SMAAEdgeDetectionRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
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

	edgeDetectionShader = ShaderProgram::createShaderProgram("Resources/Shaders/AntiAliasing/smaaEdgeDetection.vert", "Resources/Shaders/AntiAliasing/smaaEdgeDetection.frag");

	uResolutionSMAAE.create(edgeDetectionShader);

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void SMAAEdgeDetectionRenderPass::render(const Effects & _effects, GLuint _inputTexture, RenderPass **_previousRenderPass)
{
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	glClear(GL_COLOR_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _inputTexture);

	edgeDetectionShader->bind();

	uResolutionSMAAE.set(glm::vec4(1.0f / state.viewportState.width, 1.0f / state.viewportState.height, state.viewportState.width, state.viewportState.height));

	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	fullscreenTriangle->getSubMesh()->render();
}
