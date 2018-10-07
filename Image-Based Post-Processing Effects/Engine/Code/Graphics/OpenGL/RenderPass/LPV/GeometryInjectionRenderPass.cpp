#include "GeometryInjectionRenderPass.h"
#include "Graphics\Volume.h"

extern size_t VOLUME_SIZE;
extern size_t RSM_SIZE;

GeometryInjectionRenderPass::GeometryInjectionRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
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

	geometryInjectionShader = ShaderProgram::createShaderProgram("Resources/Shaders/LPV/geometryInjection.vert", "Resources/Shaders/LPV/geometryInjection.frag");

	uInvViewProjection.create(geometryInjectionShader);
	uRsmWidth.create(geometryInjectionShader);
	uGridOrigin.create(geometryInjectionShader);
	uGridSize.create(geometryInjectionShader);
	uGridSpacing.create(geometryInjectionShader);
	uLightDirection.create(geometryInjectionShader);

	std::unique_ptr<glm::vec2[]> positions = std::make_unique<glm::vec2[]>(RSM_SIZE * RSM_SIZE);

	for (size_t y = 0; y < RSM_SIZE; ++y)
	{
		for (size_t x = 0; x < RSM_SIZE; ++x)
		{
			positions[y * RSM_SIZE + x] = { x, y };
		}
	}

	// create buffers/arrays
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
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

	geometryInjectionShader->bind();
	uInvViewProjection.set(_invViewProjection);
	uRsmWidth.set(static_cast<GLint>(RSM_SIZE));
	uGridOrigin.set(_geometryVolume.origin);
	uGridSize.set(glm::vec3(_geometryVolume.dimensions));
	uGridSpacing.set(glm::vec2(_geometryVolume.spacing, 1.0f / _geometryVolume.spacing));
	uLightDirection.set(_lightDir);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _normalTexture);

	glBindVertexArray(VAO);
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(RSM_SIZE * RSM_SIZE));
}
