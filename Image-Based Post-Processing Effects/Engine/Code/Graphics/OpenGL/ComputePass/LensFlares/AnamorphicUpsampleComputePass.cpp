#include "AnamorphicUpsampleComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"

AnamorphicUpsampleComputePass::AnamorphicUpsampleComputePass(unsigned int _width, unsigned int _height)
	:width(_width),
	height(_height)
{
	upsampleShader = ShaderProgram::createShaderProgram("Resources/Shaders/LensFlares/anamorphicUpsample.comp");
}

void AnamorphicUpsampleComputePass::execute(const Effects &_effects, GLuint _prefilterTexture, GLuint *_anamorphicTextureChain, size_t _chainSize, int _lastUsedTexture, unsigned int _lastWidth)
{
	upsampleShader->bind();

	for (int i = _lastUsedTexture - 1; i >= 0; --i)
	{
		_lastWidth *= 2;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _anamorphicTextureChain[i]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, _anamorphicTextureChain[i + 1]);

		glBindImageTexture(0, _anamorphicTextureChain[i], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R11F_G11F_B10F);
		GLUtility::glDispatchComputeHelper(_lastWidth, height / 2, 1, 8, 8, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _prefilterTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _anamorphicTextureChain[0]);

	glBindImageTexture(0, _prefilterTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R11F_G11F_B10F);
	GLUtility::glDispatchComputeHelper(width, height / 2, 1, 8, 8, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void AnamorphicUpsampleComputePass::resize(unsigned int _width, unsigned int _height)
{
	width = _width;
	height = _height;
}
