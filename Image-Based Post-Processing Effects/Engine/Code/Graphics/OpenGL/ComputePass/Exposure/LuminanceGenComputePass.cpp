#include "LuminanceGenComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"

LuminanceGenComputePass::LuminanceGenComputePass(unsigned int _width, unsigned int _height)
	:m_width(_width),
	m_height(_height)
{
	m_luminanceGenShader = ShaderProgram::createShaderProgram("Resources/Shaders/Exposure/luminanceGen.comp");
}

void LuminanceGenComputePass::execute(const Effects & _effects, GLuint _colorTexture, GLuint _luminanceTexture)
{
	m_luminanceGenShader->bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _colorTexture);

	glBindImageTexture(0, _luminanceTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16F);
	GLUtility::glDispatchComputeHelper(m_width, m_height, 1, 8, 8, 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

	glBindTexture(GL_TEXTURE_2D, _luminanceTexture);
	glGenerateMipmap(GL_TEXTURE_2D);
}

void LuminanceGenComputePass::resize(unsigned int _width, unsigned int _height)
{
	m_width = _width;
	m_height = _height;
}
