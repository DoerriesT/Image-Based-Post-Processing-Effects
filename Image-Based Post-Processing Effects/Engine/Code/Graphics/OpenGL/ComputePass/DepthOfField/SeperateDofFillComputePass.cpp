#include "SeperateDofFillComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"
#include "Graphics\SampleKernel.h"
#include <glm/trigonometric.hpp>

SeperateDofFillComputePass::SeperateDofFillComputePass(unsigned int _width, unsigned int _height)
	:fillSamplesSet(false),
	width(_width),
	height(_height)
{
	fillShader = ShaderProgram::createShaderProgram("Resources/Shaders/DepthOfField/dofSeperatedFill.comp");

	for (int i = 0; i < 3 * 3; ++i)
	{
		uSampleCoordsSDOFF.push_back(fillShader->createUniform(std::string("uSampleCoords") + "[" + std::to_string(i) + "]"));
	}
}

void SeperateDofFillComputePass::execute(GLuint * _dofTextures)
{
	fillShader->bind();

	if (!fillSamplesSet)
	{
		fillSamplesSet = true;

		glm::vec2 fillSamples[3 * 3];

		unsigned int nSquareTapsSide = 3;
		float fRecipTaps = 1.0f / ((float)nSquareTapsSide - 1.0f);
		const float rotAngle = glm::radians(15.0f);
		const glm::mat2 rot = glm::mat2(glm::cos(rotAngle), -glm::sin(rotAngle), glm::sin(rotAngle), glm::cos(rotAngle));

		for (unsigned int y = 0; y < nSquareTapsSide; ++y)
		{
			for (unsigned int x = 0; x < nSquareTapsSide; ++x)
			{
				fillSamples[y * nSquareTapsSide + x] = rot * shirleyUnitSquareToDisk(glm::vec2(x * fRecipTaps, y * fRecipTaps));
			}
		}

		for (int i = 0; i < 3 * 3; ++i)
		{
			fillShader->setUniform(uSampleCoordsSDOFF[i], fillSamples[i]);
		}
	}

	glBindImageTexture(0, _dofTextures[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	glBindImageTexture(1, _dofTextures[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	GLUtility::glDispatchComputeHelper(width / 2, height / 2, 1, 8, 8, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void SeperateDofFillComputePass::resize(unsigned int _width, unsigned int _height)
{
	width = _width;
	height = _height;
}
