#include "OceanRenderPass.h"
#include "Level.h"
#include "Graphics\OpenGL\RenderData.h"
#include "Graphics\Texture.h"

OceanRenderPass::OceanRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	fbo = _fbo;
	drawBuffers = { GL_COLOR_ATTACHMENT4 };
	state.blendState.enabled = false;
	state.blendState.sFactor = GL_SRC_ALPHA;
	state.blendState.dFactor = GL_ONE_MINUS_SRC_ALPHA;
	state.cullFaceState.enabled = false;
	state.cullFaceState.face = GL_BACK;
	state.depthState.enabled = true;
	state.depthState.func = GL_LEQUAL;
	state.depthState.mask = GL_TRUE;
	state.stencilState.enabled = false;
	state.stencilState.frontFunc = state.stencilState.backFunc = GL_ALWAYS;
	state.stencilState.frontRef = state.stencilState.backRef = 1;
	state.stencilState.frontMask = state.stencilState.backMask = 0xFF;
	state.stencilState.frontOpFail = state.stencilState.backOpFail = GL_KEEP;
	state.stencilState.frontOpZfail = state.stencilState.backOpZfail = GL_KEEP;
	state.stencilState.frontOpZpass = state.stencilState.backOpZpass = GL_KEEP;

	resize(_width, _height);

	oceanShader = ShaderProgram::createShaderProgram("Resources/Shaders/Water/water.vert", "Resources/Shaders/Water/Water.frag");

	uProjectionW.create(oceanShader);
	uViewW.create(oceanShader);
	uCamPosW.create(oceanShader);
	uTexCoordShiftW.create(oceanShader);
	uUseEnvironmentW.create(oceanShader);
	uWaterLevelW.create(oceanShader);
	uLightDirW.create(oceanShader);
	uLightColorW.create(oceanShader);
}

void OceanRenderPass::render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, GLuint _displacementTexture, GLuint _normalTexture, GLuint _gridVAO, GLsizei _gridSize, bool _wireframe, RenderPass ** _previousRenderPass)
{
	resize(_renderData.resolution.first, _renderData.resolution.second);
	drawBuffers[0] = _renderData.frame % 2 ? GL_COLOR_ATTACHMENT5 : GL_COLOR_ATTACHMENT4;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	oceanShader->bind();

	uProjectionW.set(_renderData.projectionMatrix);
	uViewW.set(_renderData.viewMatrix);
	uCamPosW.set(_renderData.cameraPosition);
	uTexCoordShiftW.set(glm::vec2(-1.5, 0.75) * _renderData.time * 0.25f);
	uUseEnvironmentW.set(_level->environment.environmentProbes[0]->isValid());
	uWaterLevelW.set(_level->water.level);

	if (_level->lights.directionalLights.empty())
	{
		uLightDirW.set(glm::normalize(glm::vec3(1.0f, 1.0f, 0.0f)));
		uLightColorW.set(glm::vec3(1.5f, 0.575f, 0.5f));
	}
	else
	{
		uLightDirW.set(_level->lights.directionalLights[0]->getDirection());
		uLightColorW.set(_level->lights.directionalLights[0]->getColor());
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _normalTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _displacementTexture);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _level->environment.environmentMap->getId());

	glBindVertexArray(_gridVAO);
	glEnableVertexAttribArray(0);

	if (_wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	glDrawElements(GL_TRIANGLES, _gridSize * _gridSize * 6, GL_UNSIGNED_INT, 0);
	if (_wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}
