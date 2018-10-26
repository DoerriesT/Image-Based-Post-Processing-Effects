#include "LuminanceAdaptionComputePass.h"
#include "Engine.h"

LuminanceAdaptionComputePass::LuminanceAdaptionComputePass(unsigned int _width, unsigned int _height)
	:m_width(_width),
	m_height(_height)
{
	m_luminanceAdaptionShader = ShaderProgram::createShaderProgram("Resources/Shaders/Exposure/luminanceAdaption.comp");

	m_uTimeDelta.create(m_luminanceAdaptionShader);
	m_uTau.create(m_luminanceAdaptionShader);
}

void LuminanceAdaptionComputePass::execute(const Effects & _effects, GLuint _luminanceTexture, GLuint *_temporalLuminanceTextures, bool _currentLuminanceTexture)
{
	m_luminanceAdaptionShader->bind();
	m_uTimeDelta.set((float)Engine::getTimeDelta());
	m_uTau.set(2.5f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _temporalLuminanceTextures[!_currentLuminanceTexture]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _luminanceTexture);

	glBindImageTexture(0, _temporalLuminanceTextures[_currentLuminanceTexture], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16F);
	glDispatchCompute(1, 1, 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

void LuminanceAdaptionComputePass::resize(unsigned int _width, unsigned int _height)
{
	m_width = _width;
	m_height = _height;
}
