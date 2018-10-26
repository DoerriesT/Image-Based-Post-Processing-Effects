#include "LightPropagationRenderPass.h"
#include "Graphics\Volume.h"
#include "Graphics\Texture.h"

extern size_t VOLUME_SIZE;
extern size_t RSM_SIZE;

LightPropagationRenderPass::LightPropagationRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	m_fbo = _fbo;
	m_drawBuffers = { GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6 };
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

	m_lightPropagationShader = ShaderProgram::createShaderProgram("Resources/Shaders/LPV/lightPropagation.vert", "Resources/Shaders/LPV/lightPropagation.frag");

	m_uGridSize.create(m_lightPropagationShader);
	m_uFirstIteration.create(m_lightPropagationShader);
	m_uOcclusionAmplifier.create(m_lightPropagationShader);

	m_fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

float occAmp = 1.0f;

void LightPropagationRenderPass::render(const Volume &_lightPropagationVolume, GLuint _geometryTexture, GLuint *_redTexture, GLuint *_greenTexture, GLuint *_blueTexture, RenderPass ** _previousRenderPass)
{
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	m_lightPropagationShader->bind();

	m_uGridSize.set(glm::vec3(_lightPropagationVolume.m_dimensions));
	m_uOcclusionAmplifier.set(occAmp);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, _geometryTexture);

	GLenum firstTargets[] = { GL_COLOR_ATTACHMENT4 , GL_COLOR_ATTACHMENT5 , GL_COLOR_ATTACHMENT6 };
	GLenum secondTargets[] = { GL_COLOR_ATTACHMENT1 , GL_COLOR_ATTACHMENT2 , GL_COLOR_ATTACHMENT3 };
	GLenum *targets[] = { firstTargets, secondTargets };

	m_fullscreenTriangle->getSubMesh()->enableVertexAttribArraysPositionOnly();

	for (size_t i = 0; i < static_cast<size_t>(_lightPropagationVolume.m_dimensions.x); ++i)
	{
		glDrawBuffers(3, targets[i % 2]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _redTexture[i % 2]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, _greenTexture[i % 2]);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, _blueTexture[i % 2]);

		m_uFirstIteration.set(i == 0);

		m_fullscreenTriangle->getSubMesh()->render();
	}

}
