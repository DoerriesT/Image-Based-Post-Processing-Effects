#include "LuminanceAdaptionComputePass.h"
#include "Engine.h"

LuminanceAdaptionComputePass::LuminanceAdaptionComputePass(unsigned int _width, unsigned int _height)
	:width(_width),
	height(_height)
{
	luminanceAdaptionShader = ShaderProgram::createShaderProgram("Resources/Shaders/Exposure/luminanceAdaption.comp");

	uTimeDeltaLA.create(luminanceAdaptionShader);
	uTauLA.create(luminanceAdaptionShader);
}

void LuminanceAdaptionComputePass::execute(const Effects & _effects, GLuint _luminanceTexture, GLuint *_temporalLuminanceTextures, bool _currentLuminanceTexture)
{
	luminanceAdaptionShader->bind();
	uTimeDeltaLA.set((float)Engine::getTimeDelta());
	uTauLA.set(2.5f);

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
	width = _width;
	height = _height;
}
