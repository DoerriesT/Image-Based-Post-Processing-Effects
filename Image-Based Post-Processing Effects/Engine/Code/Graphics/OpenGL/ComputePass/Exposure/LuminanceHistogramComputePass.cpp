#include "LuminanceHistogramComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"

LuminanceHistogramComputePass::LuminanceHistogramComputePass(unsigned int _width, unsigned int _height)
	:width(_width),
	height(_height)
{
	histogramShader = ShaderProgram::createShaderProgram("Resources/Shaders/Exposure/histogram.comp");

	uParamsLH.create(histogramShader);
}

void LuminanceHistogramComputePass::execute(GLuint _colorTexture, GLuint _intermediaryTexture, const glm::vec2 &_params)
{
	histogramShader->bind();

	uParamsLH.set(_params);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _colorTexture);

	glBindImageTexture(0, _intermediaryTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	GLUtility::glDispatchComputeHelper(width / 8 + ((width % 8 == 0) ? 0 : 1), height / 8 + ((height % 8 == 0) ? 0 : 1), 1, 8, 8, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void LuminanceHistogramComputePass::resize(unsigned int _width, unsigned int _height)
{
	width = _width;
	height = _height;
}
