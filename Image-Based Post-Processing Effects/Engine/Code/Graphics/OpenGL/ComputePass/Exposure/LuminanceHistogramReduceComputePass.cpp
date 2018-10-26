#include "LuminanceHistogramReduceComputePass.h"

LuminanceHistogramReduceComputePass::LuminanceHistogramReduceComputePass(unsigned int _width, unsigned int _height)
	:m_width(_width),
	m_height(_height)
{
	m_reduceShader = ShaderProgram::createShaderProgram("Resources/Shaders/Exposure/histogramReduce.comp");

	m_uLines.create(m_reduceShader);
}

void LuminanceHistogramReduceComputePass::execute(GLuint _histogramTexture)
{
	glBindImageTexture(1, _histogramTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	m_reduceShader->bind();

	unsigned int numGroupsX = m_width / 8 + ((m_width % 8 == 0) ? 0 : 1);
	unsigned int numGroupsY = m_height / 8 + ((m_height % 8 == 0) ? 0 : 1);
	numGroupsX = numGroupsX / 8 + ((numGroupsX % 8 == 0) ? 0 : 1);
	numGroupsY = numGroupsY / 8 + ((numGroupsY % 8 == 0) ? 0 : 1);

	m_uLines.set(numGroupsX * numGroupsY);

	const unsigned int histogramBuckets = 64;

	glDispatchCompute(histogramBuckets / 4, 1, 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

void LuminanceHistogramReduceComputePass::resize(unsigned int _width, unsigned int _height)
{
	m_width = _width;
	m_height = _height;
}
