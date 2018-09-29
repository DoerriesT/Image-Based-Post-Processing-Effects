#include "SMAABlendRenderPass.h"

SMAABlendRenderPass::SMAABlendRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	fbo = _fbo;
	drawBuffers = { GL_COLOR_ATTACHMENT2 };
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

	blendShader = ShaderProgram::createShaderProgram("Resources/Shaders/AntiAliasing/smaaNeighborhoodBlending.vert", "Resources/Shaders/AntiAliasing/smaaNeighborhoodBlending.frag");

	uResolutionSMAAC.create(blendShader);

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void SMAABlendRenderPass::render(const Effects & _effects, GLuint _inputTexture, GLuint _velocityTexture, GLuint _blendWeightTexture, bool _currentSample, RenderPass ** _previousRenderPass)
{
	drawBuffers[0] = GL_COLOR_ATTACHMENT2 + static_cast<int>(_currentSample);
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _inputTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _blendWeightTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, _velocityTexture);

	blendShader->bind();

	uResolutionSMAAC.set(glm::vec4(1.0f / state.viewportState.width, 1.0f / state.viewportState.height, state.viewportState.width, state.viewportState.height));

	fullscreenTriangle->getSubMesh()->render();
}
