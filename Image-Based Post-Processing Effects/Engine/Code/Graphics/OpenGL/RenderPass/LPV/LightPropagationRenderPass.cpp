#include "LightPropagationRenderPass.h"
#include "Graphics\Volume.h"
#include "Graphics\Texture.h"

extern int VOLUME_SIZE;
extern int RSM_SIZE;

LightPropagationRenderPass::LightPropagationRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	fbo = _fbo;
	drawBuffers = { GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6 };
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

	lightPropagationShader = ShaderProgram::createShaderProgram("Resources/Shaders/LPV/lightPropagation.vert", "Resources/Shaders/LPV/lightPropagation.frag");

	uGridSize.create(lightPropagationShader);
	uFirstIteration.create(lightPropagationShader);

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void LightPropagationRenderPass::render(const Volume &_lightPropagationVolume, GLuint _geometryTexture, GLuint *_redTexture, GLuint *_greenTexture, GLuint *_blueTexture, GLuint *accumTextures, RenderPass ** _previousRenderPass)
{
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	lightPropagationShader->bind();

	uGridSize.set(glm::vec3(_lightPropagationVolume.dimensions));

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, _geometryTexture);

	GLenum firstTargets[] = { GL_COLOR_ATTACHMENT4 , GL_COLOR_ATTACHMENT5 , GL_COLOR_ATTACHMENT6 };
	GLenum secondTargets[] = { GL_COLOR_ATTACHMENT1 , GL_COLOR_ATTACHMENT2 , GL_COLOR_ATTACHMENT3 };
	GLenum *targets[] = { firstTargets, secondTargets };

	fullscreenTriangle->getSubMesh()->enableVertexAttribArraysPositionOnly();

	for (unsigned int i = 0; i < 32; ++i)
	{
		glDrawBuffers(3, targets[i % 2]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _redTexture[i % 2]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, _greenTexture[i % 2]);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, _blueTexture[i % 2]);

		uFirstIteration.set(i == 0);

		fullscreenTriangle->getSubMesh()->render();
	}

}
