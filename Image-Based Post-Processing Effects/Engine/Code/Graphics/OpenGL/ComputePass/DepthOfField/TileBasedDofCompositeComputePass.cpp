#include "TileBasedDofCompositeComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"
#include "Graphics\OpenGL\GLTimerQuery.h"

TileBasedDofCompositeComputePass::TileBasedDofCompositeComputePass(unsigned int _width, unsigned int _height)
	:m_width(_width),
	m_height(_height)
{
	m_compositeShader = ShaderProgram::createShaderProgram("Resources/Shaders/DepthOfField/dofSeperatedComposite.comp");
}

double seperateDofCompositeComputeTime;

void TileBasedDofCompositeComputePass::execute(GLuint _colorTexture, GLuint _cocTexture, GLuint _destinationTexture)
{
	SCOPED_TIMER_QUERY(seperateDofCompositeComputeTime);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _colorTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _cocTexture);

	m_compositeShader->bind();

	glBindImageTexture(0, _destinationTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	GLUtility::glDispatchComputeHelper(m_width, m_height, 1, 8, 8, 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

void TileBasedDofCompositeComputePass::resize(unsigned int _width, unsigned int _height)
{
	m_width = _width;
	m_height = _height;
}
