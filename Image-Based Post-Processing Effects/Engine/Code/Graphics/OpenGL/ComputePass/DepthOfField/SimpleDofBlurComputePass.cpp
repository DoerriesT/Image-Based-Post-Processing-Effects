#include "SimpleDofBlurComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"
#include "Graphics\SampleKernel.h"
#include "Graphics\OpenGL\GLTimerQuery.h"

SimpleDofBlurComputePass::SimpleDofBlurComputePass(unsigned int _width, unsigned int _height)
	:m_blurSamplesSet(false),
	m_width(_width),
	m_height(_height)
{
	m_blurShader = ShaderProgram::createShaderProgram("Resources/Shaders/DepthOfField/dofSimpleBlur.comp");

	for (int i = 0; i < 7 * 7; ++i)
	{
		m_uSampleCoords.push_back(m_blurShader->createUniform(std::string("uSampleCoords") + "[" + std::to_string(i) + "]"));
	}
}

double simpleDofBlurComputeTime;

void SimpleDofBlurComputePass::execute(GLuint _colorTexture, GLuint _cocTexture, GLuint * _dofTextures)
{
	SCOPED_TIMER_QUERY(simpleDofBlurComputeTime);
	m_blurShader->bind();

	if (!m_blurSamplesSet)
	{
		m_blurSamplesSet = true;

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
			m_blurShader->setUniform(m_uSampleCoords[i], blurSamples[i]);
		}
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _colorTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _cocTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, _dofTextures[0]);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, _dofTextures[1]);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, _dofTextures[2]);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, _dofTextures[3]);

	glBindImageTexture(0, _dofTextures[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	glBindImageTexture(1, _dofTextures[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	GLUtility::glDispatchComputeHelper(m_width / 2, m_height / 2, 1, 8, 8, 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

void SimpleDofBlurComputePass::resize(unsigned int _width, unsigned int _height)
{
	m_width = _width;
	m_height = _height;
}
