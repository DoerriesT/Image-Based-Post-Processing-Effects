#include "TildeH0kRenderPass.h"
#include "Level.h"
#include "Graphics/Texture.h"

TildeH0kRenderPass::TildeH0kRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	fbo = _fbo;
	drawBuffers = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
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

	tildeH0kShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Water/tildeH0k.frag");

	uSimulationResolutionH0.create(tildeH0kShader);
	uWorldSizeH0.create(tildeH0kShader);
	uWaveAmplitudeH0.create(tildeH0kShader);
	uWindDirectionH0.create(tildeH0kShader);
	uWindSpeedH0.create(tildeH0kShader);

	fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void TildeH0kRenderPass::render(const Water & _water, RenderPass ** _previousRenderPass)
{
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	std::shared_ptr<Texture> noise0;
	std::shared_ptr<Texture> noise1;
	std::shared_ptr<Texture> noise2;
	std::shared_ptr<Texture> noise3;

	// consider keeping these textures permanently in memory
	switch (_water.simulationResolution)
	{
	case 512:
		noise0 = Texture::createTexture("Resources/Textures/Noise512_0.dds", true);
		noise1 = Texture::createTexture("Resources/Textures/Noise512_1.dds", true);
		noise2 = Texture::createTexture("Resources/Textures/Noise512_2.dds", true);
		noise3 = Texture::createTexture("Resources/Textures/Noise512_3.dds", true);
		break;
	case 256:
	default:
		noise0 = Texture::createTexture("Resources/Textures/Noise256_0.dds", true);
		noise1 = Texture::createTexture("Resources/Textures/Noise256_1.dds", true);
		noise2 = Texture::createTexture("Resources/Textures/Noise256_2.dds", true);
		noise3 = Texture::createTexture("Resources/Textures/Noise256_3.dds", true);
		break;
	}

	tildeH0kShader->bind();

	uSimulationResolutionH0.set(_water.simulationResolution);
	uWorldSizeH0.set(_water.worldSize);
	uWaveAmplitudeH0.set(_water.waveAmplitude);
	uWindDirectionH0.set(_water.normalizedWindDirection);
	uWindSpeedH0.set(_water.windSpeed);
	uWaveSuppressionExpH0.set(_water.waveSuppressionExponent);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, noise0->getId());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, noise1->getId());
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, noise2->getId());
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, noise3->getId());

	fullscreenTriangle->getSubMesh()->render();
}
