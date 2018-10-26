#include "LuminanceHistogramComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"

LuminanceHistogramComputePass::LuminanceHistogramComputePass(unsigned int _width, unsigned int _height)
	:m_width(_width),
	m_height(_height)
{
	m_histogramShader = ShaderProgram::createShaderProgram("Resources/Shaders/Exposure/histogram.comp");

	m_uParams.create(m_histogramShader);
}

void LuminanceHistogramComputePass::execute(GLuint _colorTexture, GLuint _intermediaryTexture, const glm::vec2 &_params)
{
	m_histogramShader->bind();

	m_uParams.set(_params);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _colorTexture);

	glBindImageTexture(0, _intermediaryTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	GLUtility::glDispatchComputeHelper(m_width / 8 + ((m_width % 8 == 0) ? 0 : 1), m_height / 8 + ((m_height % 8 == 0) ? 0 : 1), 1, 8, 8, 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

void LuminanceHistogramComputePass::resize(unsigned int _width, unsigned int _height)
{
	m_width = _width;
	m_height = _height;
}
