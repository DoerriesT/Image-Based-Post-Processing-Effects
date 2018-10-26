#include "SMAABlendWeightRenderPass.h"
#include "Graphics\Texture.h"

SMAABlendWeightRenderPass::SMAABlendWeightRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	m_fbo = _fbo;
	m_drawBuffers = { GL_COLOR_ATTACHMENT1 };
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

	m_blendWeightShader = ShaderProgram::createShaderProgram("Resources/Shaders/AntiAliasing/smaaBlendingWeightCalculation.vert", "Resources/Shaders/AntiAliasing/smaaBlendingWeightCalculation.frag");

	m_uResolution.create(m_blendWeightShader);
	m_uTemporalSample.create(m_blendWeightShader);
	m_uTemporalAA.create(m_blendWeightShader);

	m_fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void SMAABlendWeightRenderPass::render(const Effects & _effects, GLuint _edgesTexture, bool _temporal, bool _currentSample, RenderPass ** _previousRenderPass)
{
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	static std::shared_ptr<Texture> smaaAreaLut = Texture::createTexture("Resources/Textures/AreaTexDX10.dds", true);
	static std::shared_ptr<Texture> smaaSearchLut = Texture::createTexture("Resources/Textures/SearchTex.dds", true);

	m_fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	glClear(GL_COLOR_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _edgesTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, smaaAreaLut->getId());
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, smaaSearchLut->getId());

	m_blendWeightShader->bind();

	m_uResolution.set(glm::vec4(1.0f / m_state.m_viewportState.m_width, 1.0f / m_state.m_viewportState.m_height, m_state.m_viewportState.m_width, m_state.m_viewportState.m_height));
	m_uTemporalSample.set(_currentSample);
	m_uTemporalAA.set(_temporal);

	m_fullscreenTriangle->getSubMesh()->render();
}
