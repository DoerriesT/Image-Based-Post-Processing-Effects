#include "LensFlareGenRenderPass.h"
#include "Graphics\Effects.h"
#include "Graphics\Texture.h"

LensFlareGenRenderPass::LensFlareGenRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
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

	m_lensFlareGenShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/LensFlares/lensFlareGen.frag");

	m_uGhosts.create(m_lensFlareGenShader);
	m_uGhostDispersal.create(m_lensFlareGenShader);
	m_uHaloRadius.create(m_lensFlareGenShader);
	m_uDistortion.create(m_lensFlareGenShader);
	m_uScale.create(m_lensFlareGenShader);
	m_uBias.create(m_lensFlareGenShader);

	m_fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void LensFlareGenRenderPass::render(const Effects & _effects, GLuint _inputTexture, RenderPass ** _previousRenderPass)
{
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	static std::shared_ptr<Texture> lensColorTexture = Texture::createTexture("Resources/Textures/lenscolor.dds", true);

	m_fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _inputTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(lensColorTexture->getTarget(), lensColorTexture->getId());

	m_lensFlareGenShader->bind();
	m_uGhosts.set(_effects.m_lensFlares.m_flareCount);
	m_uGhostDispersal.set(_effects.m_lensFlares.m_flareSpacing);
	m_uHaloRadius.set(_effects.m_lensFlares.m_haloWidth);
	m_uDistortion.set(_effects.m_lensFlares.m_chromaticDistortion);

	m_fullscreenTriangle->getSubMesh()->render();
}

void LensFlareGenRenderPass::resize(unsigned int _width, unsigned int _height)
{
	m_state.m_viewportState = { 0, 0, static_cast<GLint>(_width / 2), static_cast<GLint>(_height / 2) };
}
