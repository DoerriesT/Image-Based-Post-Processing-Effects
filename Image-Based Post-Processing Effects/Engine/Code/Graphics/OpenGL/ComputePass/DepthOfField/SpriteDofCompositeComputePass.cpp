#include "SpriteDofCompositeComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"
#include "Graphics\OpenGL\GLTimerQuery.h"

SpriteDofCompositeComputePass::SpriteDofCompositeComputePass(unsigned int _width, unsigned int _height)
	:m_width(_width),
	m_height(_height)
{
	m_compositeShader = ShaderProgram::createShaderProgram("Resources/Shaders/DepthOfField/dofSpriteCompose.comp");
}

double spriteDofCompositeComputeTime;

void SpriteDofCompositeComputePass::execute(GLuint _destinationTexture)
{
	SCOPED_TIMER_QUERY(spriteDofCompositeComputeTime);
	m_compositeShader->bind();

	glBindImageTexture(0, _destinationTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	GLUtility::glDispatchComputeHelper(m_width, m_height, 1, 8, 8, 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

void SpriteDofCompositeComputePass::resize(unsigned int _width, unsigned int _height)
{
	m_width = _width;
	m_height = _height;
}
