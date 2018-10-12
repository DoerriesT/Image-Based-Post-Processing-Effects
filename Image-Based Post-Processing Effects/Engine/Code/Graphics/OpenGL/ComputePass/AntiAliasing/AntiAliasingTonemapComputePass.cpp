#include "AntiAliasingTonemapComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"

AntiAliasingTonemapComputePass::AntiAliasingTonemapComputePass(unsigned int _width, unsigned int _height)
	:width(_width),
	height(_height)
{
	tonemapShader = ShaderProgram::createShaderProgram("Resources/Shaders/AntiAliasing/antiAliasingTonemap.comp");
}

void AntiAliasingTonemapComputePass::execute(GLuint _colorTexture)
{
	tonemapShader->bind();
	glBindImageTexture(0, _colorTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);
	GLUtility::glDispatchComputeHelper(width, height, 1, 8, 8, 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void AntiAliasingTonemapComputePass::resize(unsigned int _width, unsigned int _height)
{
	width = _width;
	height = _height;
}
