#include "TileBasedDofFillComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"
#include "Graphics\SampleKernel.h"
#include <glm/trigonometric.hpp>
#include "Graphics\OpenGL\GLTimerQuery.h"

TileBasedDofFillComputePass::TileBasedDofFillComputePass(unsigned int _width, unsigned int _height)
	:m_fillSamplesSet(false),
	m_width(_width),
	m_height(_height)
{
	m_fillShader = ShaderProgram::createShaderProgram("Resources/Shaders/DepthOfField/dofSeperatedFill.comp");

	for (int i = 0; i < 3 * 3; ++i)
	{
		m_uSampleCoords.push_back(m_fillShader->createUniform(std::string("uSampleCoords") + "[" + std::to_string(i) + "]"));
	}
}

double seperateDofFillComputeTime;

void TileBasedDofFillComputePass::execute(GLuint * _dofTextures)
{
	SCOPED_TIMER_QUERY(seperateDofFillComputeTime);
	m_fillShader->bind();

	if (!m_fillSamplesSet)
	{
		m_fillSamplesSet = true;

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
			m_fillShader->setUniform(m_uSampleCoords[i], fillSamples[i]);
		}
	}

	glBindImageTexture(0, _dofTextures[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	glBindImageTexture(1, _dofTextures[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	GLUtility::glDispatchComputeHelper(m_width / 2, m_height / 2, 1, 8, 8, 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

void TileBasedDofFillComputePass::resize(unsigned int _width, unsigned int _height)
{
	m_width = _width;
	m_height = _height;
}
