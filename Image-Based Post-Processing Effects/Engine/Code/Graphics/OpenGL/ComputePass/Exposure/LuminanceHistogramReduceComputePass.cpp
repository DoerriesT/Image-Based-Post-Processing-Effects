#include "LuminanceHistogramReduceComputePass.h"

LuminanceHistogramReduceComputePass::LuminanceHistogramReduceComputePass(unsigned int _width, unsigned int _height)
	:width(_width),
	height(_height)
{
	reduceShader = ShaderProgram::createShaderProgram("Resources/Shaders/Exposure/histogramReduce.comp");

	uLinesLHR.create(reduceShader);
}

void LuminanceHistogramReduceComputePass::execute(GLuint _histogramTexture)
{
	glBindImageTexture(1, _histogramTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	reduceShader->bind();

	unsigned int numGroupsX = width / 8 + ((width % 8 == 0) ? 0 : 1);
	unsigned int numGroupsY = height / 8 + ((height % 8 == 0) ? 0 : 1);
	numGroupsX = numGroupsX / 8 + ((numGroupsX % 8 == 0) ? 0 : 1);
	numGroupsY = numGroupsY / 8 + ((numGroupsY % 8 == 0) ? 0 : 1);

	uLinesLHR.set(numGroupsX * numGroupsY);

	glDispatchCompute(64 / 4, 1, 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

void LuminanceHistogramReduceComputePass::resize(unsigned int _width, unsigned int _height)
{
	width = _width;
	height = _height;
}
