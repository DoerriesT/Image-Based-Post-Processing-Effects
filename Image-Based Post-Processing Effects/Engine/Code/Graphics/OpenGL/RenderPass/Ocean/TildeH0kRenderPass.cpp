#include "TildeH0kRenderPass.h"
#include "Level.h"
#include "Graphics/Texture.h"

TildeH0kRenderPass::TildeH0kRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height)
{
	m_fbo = _fbo;
	m_drawBuffers = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
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

	m_tildeH0kShader = ShaderProgram::createShaderProgram("Resources/Shaders/Shared/fullscreenTriangle.vert", "Resources/Shaders/Ocean/tildeH0k.frag");

	m_uSimulationResolution.create(m_tildeH0kShader);
	m_uWorldSize.create(m_tildeH0kShader);
	m_uWaveAmplitude.create(m_tildeH0kShader);
	m_uWindDirection.create(m_tildeH0kShader);
	m_uWindSpeed.create(m_tildeH0kShader);

	m_fullscreenTriangle = Mesh::createMesh("Resources/Models/fullscreenTriangle.mesh", 1, true);
}

void TildeH0kRenderPass::render(const OceanParams & _water, RenderPass ** _previousRenderPass)
{
	RenderPass::begin(*_previousRenderPass);
	*_previousRenderPass = this;

	m_fullscreenTriangle->getSubMesh()->enableVertexAttribArrays();

	std::shared_ptr<Texture> noise0;
	std::shared_ptr<Texture> noise1;
	std::shared_ptr<Texture> noise2;
	std::shared_ptr<Texture> noise3;

	// consider keeping these textures permanently in memory
	switch (_water.m_simulationResolution)
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

	m_tildeH0kShader->bind();

	m_uSimulationResolution.set(_water.m_simulationResolution);
	m_uWorldSize.set(_water.m_worldSize);
	m_uWaveAmplitude.set(_water.m_waveAmplitude);
	m_uWindDirection.set(_water.m_normalizedWindDirection);
	m_uWindSpeed.set(_water.m_windSpeed);
	m_uWaveSuppressionExp.set(_water.m_waveSuppressionExponent);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, noise0->getId());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, noise1->getId());
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, noise2->getId());
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, noise3->getId());

	m_fullscreenTriangle->getSubMesh()->render();
}
