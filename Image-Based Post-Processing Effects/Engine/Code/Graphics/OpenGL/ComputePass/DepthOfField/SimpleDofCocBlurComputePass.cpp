#include "SimpleDofCocBlurComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"

SimpleDofCocBlurComputePass::SimpleDofCocBlurComputePass(unsigned int _width, unsigned int _height)
	:m_width(_width),
	m_height(_height)
{
	m_cocBlurShader = ShaderProgram::createShaderProgram("Resources/Shaders/DepthOfField/cocBlur.comp");

	m_uDirection.create(m_cocBlurShader);
}

void SimpleDofCocBlurComputePass::execute(GLuint _cocTexture, GLuint * _destinationTextures)
{
	m_cocBlurShader->bind();
	m_uDirection.set(false);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _cocTexture);

	glBindImageTexture(0, _destinationTextures[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
	GLUtility::glDispatchComputeHelper(m_width / 2, m_height / 2, 1, 8, 8, 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

	glBindTexture(GL_TEXTURE_2D, _destinationTextures[0]);

	m_uDirection.set(true);

	glBindImageTexture(0, _destinationTextures[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
	GLUtility::glDispatchComputeHelper(m_width / 2, m_height / 2, 1, 8, 8, 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

void SimpleDofCocBlurComputePass::resize(unsigned int _width, unsigned int _height)
{
	m_width = _width;
	m_height = _height;
}
