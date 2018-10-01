#include "BloomUpsampleComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"

BloomUpsampleComputePass::BloomUpsampleComputePass(unsigned int _width, unsigned int _height)
	:width(_width),
	height(_height)
{
	upsampleShader = ShaderProgram::createShaderProgram("Resources/Shaders/Bloom/upsample.comp");

	uAddPrevious.create(upsampleShader);
	uRadius.create(upsampleShader);
	uLevel.create(upsampleShader);
}

void BloomUpsampleComputePass::execute(GLuint _sourceTexture, GLuint _destinationTexture)
{
	upsampleShader->bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _sourceTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _destinationTexture);

	uAddPrevious.set(false);

	float radiusMult[] =
	{
		1.3f, 1.25f, 1.2f, 1.15f, 1.1f, 1.05f
	};

	for (unsigned int i = 0; i < 6; ++i)
	{
		if (i == 1)
		{
			uAddPrevious.set(true);
		}

		float resolutionFactor = (float)std::pow(0.5f, 6 - i);
		unsigned int w = (unsigned int)(width * resolutionFactor);
		unsigned int h = (unsigned int)(height * resolutionFactor);

		uLevel.set(5 - i);
		uRadius.set(radiusMult[i]);

		glBindImageTexture(0, _destinationTexture, 5 - i, GL_FALSE, 0, GL_WRITE_ONLY, GL_R11F_G11F_B10F);
		GLUtility::glDispatchComputeHelper(w, h, 1, 8, 8, 1);
		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	}
}

void BloomUpsampleComputePass::resize(unsigned int _width, unsigned int _height)
{
	width = _width;
	height = _height;
}
