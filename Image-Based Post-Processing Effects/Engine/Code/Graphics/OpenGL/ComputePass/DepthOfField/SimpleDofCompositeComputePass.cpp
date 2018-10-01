#include "SimpleDofCompositeComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"

SimpleDofCompositeComputePass::SimpleDofCompositeComputePass(unsigned int _width, unsigned int _height)
	:width(_width),
	height(_height)
{
	compositeShader = ShaderProgram::createShaderProgram("Resources/Shaders/DepthOfField/dofSimpleComposite.comp");
}

void SimpleDofCompositeComputePass::execute(GLuint _destinationTexture)
{
	compositeShader->bind();

	glBindImageTexture(0, _destinationTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	GLUtility::glDispatchComputeHelper(width, height, 1, 8, 8, 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

void SimpleDofCompositeComputePass::resize(unsigned int _width, unsigned int _height)
{
	width = _width;
	height = _height;
}
