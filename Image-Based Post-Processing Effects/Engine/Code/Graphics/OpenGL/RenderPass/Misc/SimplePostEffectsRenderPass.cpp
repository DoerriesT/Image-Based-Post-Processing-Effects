#include "SimplePostEffectsRenderPass.h"
#include "Engine.h"
#include "Graphics\Effects.h"

SimplePostEffectsRenderPass::SimplePostEffectsRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
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

	simplePostEffectsShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Misc/singlePassEffects.frag");

	uTimeS.create(simplePostEffectsShader);
	uFilmGrainStrengthS.create(simplePostEffectsShader);
	uVignetteS.create(simplePostEffectsShader);
	uFilmGrainS.create(simplePostEffectsShader);
	uChromaticAberrationS.create(simplePostEffectsShader);
	uChromAbOffsetMultiplierS.create(simplePostEffectsShader);

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void SimplePostEffectsRenderPass::render(const Effects & _effects, GLuint _inputTexture, GLenum _drawBuffer, RenderPass ** _previousRenderPass)
{
	drawBuffers[0] = _drawBuffer;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _inputTexture);

	simplePostEffectsShader->bind();
	uTimeS.set((float)Engine::getTime());
	uFilmGrainStrengthS.set(_effects.filmGrain.strength);
	uVignetteS.set(_effects.vignette.enabled);
	uFilmGrainS.set(_effects.filmGrain.enabled);
	uChromaticAberrationS.set(_effects.chromaticAberration.enabled);
	uChromAbOffsetMultiplierS.set(_effects.chromaticAberration.offsetMultiplier);

	fullscreenTriangle->getSubMesh()->render();
}