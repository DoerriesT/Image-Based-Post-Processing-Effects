#include "LuminanceHistogramAdaptionComputePass.h"
#include "Engine.h"

LuminanceHistogramAdaptionComputePass::LuminanceHistogramAdaptionComputePass(unsigned int _width, unsigned int _height)
	:m_width(_width),
	m_height(_height)
{
	m_adaptionShader = ShaderProgram::createShaderProgram("Resources/Shaders/Exposure/histogramAdaption.comp");

	m_uTimeDelta.create(m_adaptionShader);
	m_uTau.create(m_adaptionShader);
	m_uParams.create(m_adaptionShader);
}

void LuminanceHistogramAdaptionComputePass::execute(GLuint _histogramTexture, GLuint * _temporalLuminanceTextures, bool _currentLuminanceTexture, const glm::vec2 & _params)
{
	m_adaptionShader->bind();
	m_uTimeDelta.set((float)Engine::getTimeDelta());
	m_uTau.set(2.5f);
	m_uParams.set(_params);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _temporalLuminanceTextures[!_currentLuminanceTexture]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _histogramTexture);

	glBindImageTexture(0, _temporalLuminanceTextures[_currentLuminanceTexture], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16F);
	glDispatchCompute(1, 1, 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

void LuminanceHistogramAdaptionComputePass::resize(unsigned int _width, unsigned int _height)
{
	m_width = _width;
	m_height = _height;
}
