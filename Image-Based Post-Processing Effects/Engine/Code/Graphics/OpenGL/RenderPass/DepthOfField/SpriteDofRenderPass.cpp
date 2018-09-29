#include "SpriteDofRenderPass.h"
#include "Graphics\Texture.h"

SpriteDofRenderPass::SpriteDofRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	fbo = _fbo;
	drawBuffers = { GL_COLOR_ATTACHMENT0 };
	state.blendState.enabled = true;
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

	spriteShader = ShaderProgram::createShaderProgram("Resources/Shaders/DepthOfField/dofSprite.vert", "Resources/Shaders/DepthOfField/dofSprite.frag");

	uWidthDOF.create(spriteShader);
	uHeightDOF.create(spriteShader);

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);

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
	glGenVertexArrays(1, &spriteVAO);
	glGenBuffers(1, &spriteVBO);
	glGenBuffers(1, &spriteEBO);
	glBindVertexArray(spriteVAO);
	glBindBuffer(GL_ARRAY_BUFFER, spriteVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(spritePositions), spritePositions, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, spriteEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(spriteIndices), spriteIndices, GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);

	glBindVertexArray(0);
}

void SpriteDofRenderPass::render(GLuint _colorTexture, GLuint _depthTexture, GLuint _cocTexture, GLuint _destinationTexture, RenderPass ** _previousRenderPass)
{
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

	glBindVertexArray(spriteVAO);
	glEnableVertexAttribArray(0);

	spriteShader->bind();

	uWidthDOF.set(state.viewportState.width / 2);
	uHeightDOF.set(state.viewportState.height);

	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL, (state.viewportState.width / 2) * state.viewportState.height);
}
