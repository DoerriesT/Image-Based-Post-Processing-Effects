#include "GeometryInjectionRenderPass.h"
#include "Graphics\Volume.h"

extern size_t VOLUME_SIZE;
extern size_t RSM_SIZE;

GeometryInjectionRenderPass::GeometryInjectionRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	m_fbo = _fbo;
	m_drawBuffers = { GL_COLOR_ATTACHMENT0 };
	m_state.m_blendState.m_enabled = true;
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

	m_geometryInjectionShader = ShaderProgram::createShaderProgram("Resources/Shaders/LPV/geometryInjection.vert", "Resources/Shaders/LPV/geometryInjection.frag");

	m_uInvViewProjection.create(m_geometryInjectionShader);
	m_uRsmWidth.create(m_geometryInjectionShader);
	m_uGridOrigin.create(m_geometryInjectionShader);
	m_uGridSize.create(m_geometryInjectionShader);
	m_uGridSpacing.create(m_geometryInjectionShader);
	m_uLightDirection.create(m_geometryInjectionShader);

	std::unique_ptr<glm::vec2[]> positions = std::make_unique<glm::vec2[]>(RSM_SIZE * RSM_SIZE);

	for (size_t y = 0; y < RSM_SIZE; ++y)
	{
		for (size_t x = 0; x < RSM_SIZE; ++x)
		{
			positions[y * RSM_SIZE + x] = { x, y };
		}
	}

	// create buffers/arrays
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, RSM_SIZE * RSM_SIZE * sizeof(glm::vec2), positions.get(), GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);

	glBindVertexArray(0);
}

void GeometryInjectionRenderPass::render(const Volume & _geometryVolume, const glm::mat4 & _invViewProjection, GLint _normalTexture, const glm::vec3 &_lightDir, RenderPass ** _previousRenderPass)
{
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glPointSize(1.0f);

	m_geometryInjectionShader->bind();
	m_uInvViewProjection.set(_invViewProjection);
	m_uRsmWidth.set(static_cast<GLint>(RSM_SIZE));
	m_uGridOrigin.set(_geometryVolume.m_origin);
	m_uGridSize.set(glm::vec3(_geometryVolume.m_dimensions));
	m_uGridSpacing.set(glm::vec2(_geometryVolume.m_spacing, 1.0f / _geometryVolume.m_spacing));
	m_uLightDirection.set(_lightDir);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _normalTexture);

	glBindVertexArray(m_VAO);
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(RSM_SIZE * RSM_SIZE));
}
