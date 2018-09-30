#include "SeperateDofBlurComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"
#include "Graphics\SampleKernel.h"

SeperateDofBlurComputePass::SeperateDofBlurComputePass(unsigned int _width, unsigned int _height)
	:blurSamplesSet(false),
	width(_width),
	height(_height)
{
	blurShader = ShaderProgram::createShaderProgram("Resources/Shaders/DepthOfField/dofSeperatedBlur.comp");

	for (int i = 0; i < 7 * 7; ++i)
	{
		uSampleCoordsSDOFB.push_back(blurShader->createUniform(std::string("uSampleCoords") + "[" + std::to_string(i) + "]"));
	}
}

void SeperateDofBlurComputePass::execute(GLuint *_dofTextures, GLuint _cocTexture, GLuint _cocTileTexture)
{
	blurShader->bind();

	if (!blurSamplesSet)
	{
		blurSamplesSet = true;

		glm::vec2 blurSamples[7 * 7];

		unsigned int nSquareTapsSide = 7;
		float fRecipTaps = 1.0f / ((float)nSquareTapsSide - 1.0f);

		for (unsigned int y = 0; y < nSquareTapsSide; ++y)
		{
			for (unsigned int x = 0; x < nSquareTapsSide; ++x)
			{
				blurSamples[y * nSquareTapsSide + x] = shirleyUnitSquareToDisk(glm::vec2(x * fRecipTaps, y * fRecipTaps));
			}
		}

		for (int i = 0; i < 7 * 7; ++i)
		{
			blurShader->setUniform(uSampleCoordsSDOFB[i], blurSamples[i]);
		}
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _cocTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _cocTileTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, _dofTextures[0]);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, _dofTextures[1]);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, _dofTextures[2]);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, _dofTextures[3]);

	glBindImageTexture(0, _dofTextures[2], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	glBindImageTexture(1, _dofTextures[3], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	GLUtility::glDispatchComputeHelper(width / 2, height / 2, 1, 8, 8, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void SeperateDofBlurComputePass::resize(unsigned int _width, unsigned int _height)
{
	width = _width;
	height = _height;
}
