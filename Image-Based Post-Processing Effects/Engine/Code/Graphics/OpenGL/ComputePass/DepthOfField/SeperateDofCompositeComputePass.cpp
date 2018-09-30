#include "SeperateDofCompositeComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"

SeperateDofCompositeComputePass::SeperateDofCompositeComputePass(unsigned int _width, unsigned int _height)
	:width(_width),
	height(_height)
{
	compositeShader = ShaderProgram::createShaderProgram("Resources/Shaders/DepthOfField/dofSeperatedComposite.comp");
}

void SeperateDofCompositeComputePass::execute(GLuint _colorTexture, GLuint _cocTexture, GLuint _destinationTexture)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _colorTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _cocTexture);

	compositeShader->bind();

	glBindImageTexture(0, _destinationTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	GLUtility::glDispatchComputeHelper(width, height, 1, 8, 8, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void SeperateDofCompositeComputePass::resize(unsigned int _width, unsigned int _height)
{
	width = _width;
	height = _height;
}
