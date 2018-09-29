#include "SimpleDofBlurComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"

SimpleDofBlurComputePass::SimpleDofBlurComputePass(unsigned int _width, unsigned int _height)
	:blurSamplesSet(false),
	width(_width),
	height(_height)
{
	blurShader = ShaderProgram::createShaderProgram("Resources/Shaders/DepthOfField/dofSimpleBlur.comp");

	for (int i = 0; i < 7 * 7; ++i)
	{
		uSampleCoordsDOFB.push_back(blurShader->createUniform(std::string("uSampleCoords") + "[" + std::to_string(i) + "]"));
	}
}

glm::vec2 generateBlurSample(const glm::vec2 &_origin)
{
	float max_fstops = 8;
	float min_fstops = 1;
	float normalizedStops = 1.0f; //clamp_tpl((fstop - max_fstops) / (max_fstops - min_fstops), 0.0f, 1.0f);

	float phi;
	float r;
	const float a = 2 * _origin.x - 1;
	const float b = 2 * _origin.y - 1;
	if (abs(a) > abs(b)) // Use squares instead of absolute values
	{
		r = a;
		phi = (glm::pi<float>() / 4.0f) * (b / (a + 1e-6f));
	}
	else
	{
		r = b;
		phi = (glm::pi<float>() / 2.0f) - (glm::pi<float>() / 4.0f) * (a / (b + 1e-6f));
	}

	float rr = r;
	rr = abs(rr) * (rr > 0.0f ? 1.0f : -1.0f);

	//normalizedStops *= -0.4f * PI;
	return glm::vec2(rr * cosf(phi + normalizedStops), rr * sinf(phi + normalizedStops));
}

void SimpleDofBlurComputePass::execute(GLuint _colorTexture, GLuint _cocTexture, GLuint * _dofTextures)
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
				blurSamples[y * nSquareTapsSide + x] = generateBlurSample(glm::vec2(x * fRecipTaps, y * fRecipTaps));
			}
		}

		for (int i = 0; i < 7 * 7; ++i)
		{
			blurShader->setUniform(uSampleCoordsDOFB[i], blurSamples[i]);
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
	GLUtility::glDispatchComputeHelper(width / 2, height / 2, 1, 8, 8, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void SimpleDofBlurComputePass::resize(unsigned int _width, unsigned int _height)
{
	width = _width;
	height = _height;
}
