#include "SimpleDofFillComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"
#include "Graphics\SampleKernel.h"

SimpleDofFillComputePass::SimpleDofFillComputePass(unsigned int _width, unsigned int _height)
	:fillSamplesSet(false),
	width(_width),
	height(_height)
{
	fillShader = ShaderProgram::createShaderProgram("Resources/Shaders/DepthOfField/dofSimpleFill.comp");

	for (int i = 0; i < 3 * 3; ++i)
	{
		uSampleCoordsDOFF.push_back(fillShader->createUniform(std::string("uSampleCoords") + "[" + std::to_string(i) + "]"));
	}
}

void SimpleDofFillComputePass::execute(GLuint *_resultTextures)
{
	fillShader->bind();

	if (!fillSamplesSet)
	{
		fillSamplesSet = true;

		glm::vec2 fillSamples[3 * 3];

		unsigned int nSquareTapsSide = 3;
		float fRecipTaps = 1.0f / ((float)nSquareTapsSide - 1.0f);

		for (unsigned int y = 0; y < nSquareTapsSide; ++y)
		{
			for (unsigned int x = 0; x < nSquareTapsSide; ++x)
			{
				fillSamples[y * nSquareTapsSide + x] = shirleyUnitSquareToDisk(glm::vec2(x * fRecipTaps, y * fRecipTaps));
			}
		}

		for (int i = 0; i < 3 * 3; ++i)
		{
			fillShader->setUniform(uSampleCoordsDOFF[i], fillSamples[i]);
		}
	}

	glBindImageTexture(0, _resultTextures[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	glBindImageTexture(1, _resultTextures[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	GLUtility::glDispatchComputeHelper(width / 2, height / 2, 1, 8, 8, 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

void SimpleDofFillComputePass::resize(unsigned int _width, unsigned int _height)
{
	width = _width;
	height = _height;
}
