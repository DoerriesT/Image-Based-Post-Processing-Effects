#include "BloomDownsampleComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"

BloomDownsampleComputePass::BloomDownsampleComputePass(unsigned int _width, unsigned int _height)
	:width(_width),
	height(_height)
{
	downsampleShader = ShaderProgram::createShaderProgram("Resources/Shaders/Bloom/downsample.comp");

	uLevel.create(downsampleShader);
}

void BloomDownsampleComputePass::execute(GLuint _colorTexture, GLuint _destinationTexture)
{
	downsampleShader->bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _colorTexture);

	unsigned int w = width;
	unsigned int h = height;
	for (unsigned int i = 0; i < 6; ++i)
	{
		w /= 2;
		h /= 2;

		if (i == 1)
		{
			glBindTexture(GL_TEXTURE_2D, _destinationTexture);
		}

		// compensate for destination texture being half res
		uLevel.set(i == 0 ? 0 : i - 1);

		glBindImageTexture(0, _destinationTexture, i, GL_FALSE, 0, GL_WRITE_ONLY, GL_R11F_G11F_B10F);
		GLUtility::glDispatchComputeHelper(w, h, 1, 8, 8, 1);
		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	}
	
}

void BloomDownsampleComputePass::resize(unsigned int _width, unsigned int _height)
{
	width = _width;
	height = _height;
}
