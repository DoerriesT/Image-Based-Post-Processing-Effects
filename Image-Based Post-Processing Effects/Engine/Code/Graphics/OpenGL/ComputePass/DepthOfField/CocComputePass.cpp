#include "CocComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"

CocComputePass::CocComputePass(unsigned int _width, unsigned int _height)
	:m_width(_width),
	m_height(_height)
{
	m_cocShader = ShaderProgram::createShaderProgram("Resources/Shaders/DepthOfField/coc.comp");

	m_uFocalLength.create(m_cocShader);
	m_uApertureSize.create(m_cocShader);
	m_uNearFar.create(m_cocShader);
}

void CocComputePass::execute(GLuint _depthTexture, GLuint _destinationCocTexture, float _fieldOfView, float _nearPlane, float _farPlane)
{
	m_cocShader->bind();

	const float filmWidth = 0.035f;
	const float apertureSize = 2.4f;

	float focalLength = (0.5f * filmWidth) / glm::tan(_fieldOfView * 0.5f);

	m_uFocalLength.set(focalLength);
	m_uApertureSize.set(8.0f);
	m_uNearFar.set(glm::vec2(_nearPlane, _farPlane));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _depthTexture);

	glBindImageTexture(0, _destinationCocTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
	GLUtility::glDispatchComputeHelper(m_width, m_height, 1, 8, 8, 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

void CocComputePass::resize(unsigned int _width, unsigned int _height)
{
	m_width = _width;
	m_height = _height;
}
