#include "SimpleDofFillComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"

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

glm::vec2 generateFillSample(const glm::vec2 &_origin)
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
				fillSamples[y * nSquareTapsSide + x] = generateFillSample(glm::vec2(x * fRecipTaps, y * fRecipTaps));
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
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void SimpleDofFillComputePass::resize(unsigned int _width, unsigned int _height)
{
	width = _width;
	height = _height;
}
