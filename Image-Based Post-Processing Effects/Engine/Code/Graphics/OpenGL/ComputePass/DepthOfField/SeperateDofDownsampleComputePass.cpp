#include "SeperateDofDownsampleComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"
#include "Graphics\OpenGL\GLTimerQuery.h"

SeperateDofDownsampleComputePass::SeperateDofDownsampleComputePass(unsigned int _width, unsigned int _height)
	:m_width(_width),
	m_height(_height)
{
	m_downsampleShader = ShaderProgram::createShaderProgram("Resources/Shaders/DepthOfField/dofSeperatedDownsample.comp");
}

double seperateDofDownsampleComputeTime;

void SeperateDofDownsampleComputePass::execute(GLuint _colorTexture, GLuint _cocTexture, GLuint _destinationCocTexture, GLuint _destinationNearTexture, GLuint _destinationFarTexture)
{
	GLTimerQuery timer(seperateDofDownsampleComputeTime);
	m_downsampleShader->bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _colorTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _cocTexture);

	glBindImageTexture(0, _destinationCocTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
	glBindImageTexture(1, _destinationNearTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	glBindImageTexture(2, _destinationFarTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	GLUtility::glDispatchComputeHelper(m_width / 2, m_height / 2, 1, 8, 8, 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

void SeperateDofDownsampleComputePass::resize(unsigned int _width, unsigned int _height)
{
	m_width = _width;
	m_height = _height;
}
