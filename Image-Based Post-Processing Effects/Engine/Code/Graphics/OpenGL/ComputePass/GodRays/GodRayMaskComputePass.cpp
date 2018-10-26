#include "GodRayMaskComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"

GodRayMaskComputePass::GodRayMaskComputePass(unsigned int _width, unsigned int _height)
	:m_width(_width),
	m_height(_height)
{
	m_godRayMaskShader = ShaderProgram::createShaderProgram("Resources/Shaders/GodRays/godRayMask.comp");
}

void GodRayMaskComputePass::execute(const Effects &_effects, GLuint _colorTexture, GLuint _depthTexture, GLuint _godRayTexture)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _depthTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _colorTexture);

	m_godRayMaskShader->bind();

	glBindImageTexture(0, _godRayTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
	GLUtility::glDispatchComputeHelper(m_width / 2, m_height / 2, 1, 8, 8, 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

void GodRayMaskComputePass::resize(unsigned int _width, unsigned int _height)
{
	m_width = _width;
	m_height = _height;
}
