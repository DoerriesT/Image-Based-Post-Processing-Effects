#include "SimpleDofCocBlurComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"

SimpleDofCocBlurComputePass::SimpleDofCocBlurComputePass(unsigned int _width, unsigned int _height)
	:width(_width),
	height(_height)
{
	cocBlurShader = ShaderProgram::createShaderProgram("Resources/Shaders/DepthOfField/cocBlur.comp");

	uDirectionCOCB.create(cocBlurShader);
}

void SimpleDofCocBlurComputePass::execute(GLuint _cocTexture, GLuint * _destinationTextures)
{
	cocBlurShader->bind();
	uDirectionCOCB.set(false);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _cocTexture);

	glBindImageTexture(0, _destinationTextures[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
	GLUtility::glDispatchComputeHelper(width / 2, height / 2, 1, 8, 8, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	glBindTexture(GL_TEXTURE_2D, _destinationTextures[0]);

	uDirectionCOCB.set(true);

	glBindImageTexture(0, _destinationTextures[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
	GLUtility::glDispatchComputeHelper(width / 2, height / 2, 1, 8, 8, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void SimpleDofCocBlurComputePass::resize(unsigned int _width, unsigned int _height)
{
	width = _width;
	height = _height;
}
