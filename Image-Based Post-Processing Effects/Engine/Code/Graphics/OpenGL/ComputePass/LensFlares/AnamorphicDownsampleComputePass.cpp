#include "AnamorphicDownsampleComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"

AnamorphicDownsampleComputePass::AnamorphicDownsampleComputePass(unsigned int _width, unsigned int _height)
	:m_width(_width),
	m_height(_height)
{
	m_downsampleShader = ShaderProgram::createShaderProgram("Resources/Shaders/LensFlares/anamorphicDownsample.comp");
}

void AnamorphicDownsampleComputePass::execute(const Effects & _effects, GLuint _prefilterTexture, GLuint *_anamorphicTextureChain, size_t _chainSize, size_t &_lastUsedTexture, unsigned int &_lastWidth)
{
	m_downsampleShader->bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _prefilterTexture);

	_lastWidth = m_width;

	for (size_t i = 0; i < _chainSize && _lastWidth > 16; ++i)
	{
		_lastWidth /= 2;

		glBindImageTexture(0, _anamorphicTextureChain[i], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R11F_G11F_B10F);
		GLUtility::glDispatchComputeHelper(_lastWidth, m_height / 2, 1, 8, 8, 1);
		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _anamorphicTextureChain[i]);

		_lastUsedTexture = i;
	}
}

void AnamorphicDownsampleComputePass::resize(unsigned int _width, unsigned int _height)
{
	m_width = _width;
	m_height = _height;
}
