#include "AntiAliasingReverseTonemapComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"

AntiAliasingReverseTonemapComputePass::AntiAliasingReverseTonemapComputePass(unsigned int _width, unsigned int _height)
	:width(_width),
	height(_height)
{
	reverseTonemapShader = ShaderProgram::createShaderProgram("Resources/Shaders/AntiAliasing/antiAliasingReverseTonemap.comp");
}

void AntiAliasingReverseTonemapComputePass::execute(GLuint _colorTexture)
{
	reverseTonemapShader->bind();
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glBindImageTexture(0, _colorTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
	GLUtility::glDispatchComputeHelper(width, height, 1, 8, 8, 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void AntiAliasingReverseTonemapComputePass::resize(unsigned int _width, unsigned int _height)
{
	width = _width;
	height = _height;
}
