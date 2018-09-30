#include "LuminanceHistogramAdaptionComputePass.h"
#include "Engine.h"

LuminanceHistogramAdaptionComputePass::LuminanceHistogramAdaptionComputePass(unsigned int _width, unsigned int _height)
	:width(_width),
	height(_height)
{
	adaptionShader = ShaderProgram::createShaderProgram("Resources/Shaders/Exposure/histogramAdaption.comp");

	uTimeDeltaLHA.create(adaptionShader);
	uTauLHA.create(adaptionShader);
	uParamsLHA.create(adaptionShader);
}

void LuminanceHistogramAdaptionComputePass::execute(GLuint _histogramTexture, GLuint * _temporalLuminanceTextures, bool _currentLuminanceTexture, const glm::vec2 & _params)
{
	adaptionShader->bind();
	uTimeDeltaLHA.set((float)Engine::getTimeDelta());
	uTauLHA.set(2.5f);
	uParamsLHA.set(_params);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _temporalLuminanceTextures[!_currentLuminanceTexture]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _histogramTexture);

	glBindImageTexture(0, _temporalLuminanceTextures[_currentLuminanceTexture], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16F);
	glDispatchCompute(1, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void LuminanceHistogramAdaptionComputePass::resize(unsigned int _width, unsigned int _height)
{
	width = _width;
	height = _height;
}
