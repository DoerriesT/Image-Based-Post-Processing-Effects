#include "SpriteDofRenderPass.h"
#include "Graphics\Texture.h"
#include "Graphics\OpenGL\GLTimerQuery.h"

SpriteDofRenderPass::SpriteDofRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
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

	m_spriteShader = ShaderProgram::createShaderProgram("Resources/Shaders/DepthOfField/dofSprite.vert", "Resources/Shaders/DepthOfField/dofSprite.frag");

	m_uWidth.create(m_spriteShader);
	m_uHeight.create(m_spriteShader);

	// sprite

	glm::vec2 spritePositions[] =
	{
		glm::vec2(-1.0, -1.0),
		glm::vec2(1.0, -1.0),
		glm::vec2(-1.0, 1.0),
		glm::vec2(1.0, 1.0)
	};

	uint32_t spriteIndices[] =
	{
		0, 1, 2, 1, 3, 2
	};

	// create buffers/arrays
	glGenVertexArrays(1, &m_spriteVAO);
	glGenBuffers(1, &m_spriteVBO);
	glGenBuffers(1, &m_spriteEBO);
	glBindVertexArray(m_spriteVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_spriteVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(spritePositions), spritePositions, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_spriteEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(spriteIndices), spriteIndices, GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);

	glBindVertexArray(0);
}

double spriteDofRenderTime;

void SpriteDofRenderPass::render(GLuint _colorTexture, GLuint _depthTexture, GLuint _cocTexture, GLuint _destinationTexture, RenderPass ** _previousRenderPass)
{
	SCOPED_TIMER_QUERY(spriteDofRenderTime);
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	static std::shared_ptr<Texture> bokehSprite = Texture::createTexture("Resources/Textures/bokehsprite.dds", true);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _cocTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _colorTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, _depthTexture);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, _destinationTexture);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, bokehSprite->getId());

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _destinationTexture, 0);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glBindVertexArray(m_spriteVAO);
	glEnableVertexAttribArray(0);

	m_spriteShader->bind();

	m_uWidth.set(m_state.m_viewportState.m_width / 2);
	m_uHeight.set(m_state.m_viewportState.m_height);

	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL, (m_state.m_viewportState.m_width / 2) * m_state.m_viewportState.m_height);
}
