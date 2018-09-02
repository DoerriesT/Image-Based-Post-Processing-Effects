#include "OceanTesselationRenderPass.h"
#include "Level.h"
#include "Graphics\OpenGL\RenderData.h"
#include "Graphics\Texture.h"
#include "Graphics\OpenGL\TileRing.h"
#include "Engine.h"

OceanTesselationRenderPass::OceanTesselationRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
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

	oceanTesselationShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/terrain.vert", "Resources/Shaders/Water/water.frag", "Resources/Shaders/Shared/terrain.tessc", "Resources/Shaders/Shared/terrain.tesse");

	uViewProjectionWT.create(oceanTesselationShader);
	uProjectionWT.create(oceanTesselationShader);
	uViewWT.create(oceanTesselationShader);
	uCamPosWT.create(oceanTesselationShader);
	uTexCoordShiftWT.create(oceanTesselationShader);
	uUseEnvironmentWT.create(oceanTesselationShader);
	uWaterLevelWT.create(oceanTesselationShader);
	uLightDirWT.create(oceanTesselationShader);
	uLightColorWT.create(oceanTesselationShader);
	uTileSizeWT.create(oceanTesselationShader);
	uViewDirWT.create(oceanTesselationShader);
	uScreenSizeWT.create(oceanTesselationShader);
	uTesselatedTriWidthWT.create(oceanTesselationShader);
	uTexCoordScaleWT.create(oceanTesselationShader);
	uDisplacementScaleWT.create(oceanTesselationShader);
	uPerlinMovement.create(oceanTesselationShader);

	perlinNoiseTexture = Texture::createTexture("Resources/Textures/perlin_noise.dds", true);
}

void OceanTesselationRenderPass::render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, GLuint _displacementTexture, GLuint _normalTexture, TileRing **_tileRings, bool _wireframe, RenderPass ** _previousRenderPass)
{
	resize(_renderData.resolution.first, _renderData.resolution.second);
	drawBuffers[0] = _renderData.frame % 2 ? GL_COLOR_ATTACHMENT5 : GL_COLOR_ATTACHMENT4;
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	oceanTesselationShader->bind();
	uViewProjectionWT.set(_renderData.viewProjectionMatrix);
	uProjectionWT.set(_renderData.projectionMatrix);
	uViewWT.set(_renderData.viewMatrix);
	uCamPosWT.set(_renderData.cameraPosition);
	uTexCoordShiftWT.set(glm::vec2(-1.5, 0.75) * _renderData.time * 0.25f);
	uUseEnvironmentWT.set(_level->environment.environmentProbes[0]->isValid());
	uWaterLevelWT.set(_level->water.level);

	if (_level->lights.directionalLights.empty())
	{
		uLightDirWT.set(glm::normalize(glm::vec3(1.0f, 1.0f, 0.0f)));
		uLightColorWT.set(glm::vec3(1.5f, 0.575f, 0.5f));
	}
	else
	{
		uLightDirWT.set(_level->lights.directionalLights[0]->getDirection());
		uLightColorWT.set(_level->lights.directionalLights[0]->getColor());
	}

	uViewDirWT.set(_renderData.viewDirection);
	uScreenSizeWT.set(glm::vec2(_renderData.resolution.first, _renderData.resolution.second));
	uTesselatedTriWidthWT.set(20);
	uTexCoordScaleWT.set(1.0f / 20.0f);
	uDisplacementScaleWT.set(1.0f);

	uPerlinMovement.set(_level->water.normalizedWindDirection * float(Engine::getTime()) * -0.01f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _normalTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _displacementTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, perlinNoiseTexture->getId());
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _level->environment.environmentMap->getId());

	uTileSizeWT.set(1.0f);

	if (_wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	for (int i = 0; i < 6; ++i)
	{
		uTileSizeWT.set(_tileRings[i]->getTileSize());
		_tileRings[i]->render();
	}
	if (_wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}
