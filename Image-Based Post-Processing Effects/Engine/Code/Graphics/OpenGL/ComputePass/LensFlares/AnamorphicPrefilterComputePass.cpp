#include "AnamorphicPrefilterComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"

AnamorphicPrefilterComputePass::AnamorphicPrefilterComputePass(unsigned int _width, unsigned int _height)
	:width(_width),
	height(_height)
{
	prefilterShader = ShaderProgram::createShaderProgram("Resources/Shaders/LensFlares/anamorphicPrefilter.comp");
}

void AnamorphicPrefilterComputePass::execute(const Effects &_effects, GLuint _sourceTexture, GLuint _prefilterTexture)
{
	prefilterShader->bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _sourceTexture);

	glBindImageTexture(0, _prefilterTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R11F_G11F_B10F);
	GLUtility::glDispatchComputeHelper(width, height / 2, 1, 8, 8, 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

void AnamorphicPrefilterComputePass::resize(unsigned int _width, unsigned int _height)
{
	width = _width;
	height = _height;
}
