#include "SMAABlendWeightRenderPass.h"
#include "Graphics\Texture.h"

SMAABlendWeightRenderPass::SMAABlendWeightRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	fbo = _fbo;
	drawBuffers = { GL_COLOR_ATTACHMENT1 };
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

	blendWeightShader = ShaderProgram::createShaderProgram("Resources/Shaders/AntiAliasing/smaaBlendingWeightCalculation.vert", "Resources/Shaders/AntiAliasing/smaaBlendingWeightCalculation.frag");

	uResolutionSMAAB.create(blendWeightShader);
	uTemporalSampleSMAAB.create(blendWeightShader);
	uTemporalAASMAAB.create(blendWeightShader);

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void SMAABlendWeightRenderPass::render(const Effects & _effects, GLuint _edgesTexture, bool _temporal, bool _currentSample, RenderPass ** _previousRenderPass)
{
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	static std::shared_ptr<Texture> smaaAreaLut = Texture::createTexture("Resources/Textures/AreaTexDX10.dds", true);
	static std::shared_ptr<Texture> smaaSearchLut = Texture::createTexture("Resources/Textures/SearchTex.dds", true);

	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	glClear(GL_COLOR_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _edgesTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, smaaAreaLut->getId());
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, smaaSearchLut->getId());

	blendWeightShader->bind();

	uResolutionSMAAB.set(glm::vec4(1.0f / state.viewportState.width, 1.0f / state.viewportState.height, state.viewportState.width, state.viewportState.height));
	uTemporalSampleSMAAB.set(_currentSample);
	uTemporalAASMAAB.set(_temporal);

	fullscreenTriangle->getSubMesh()->render();
}
