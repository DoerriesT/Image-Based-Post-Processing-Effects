#include "CombinedDofPrepareComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"

CombinedDofPrepareComputePass::CombinedDofPrepareComputePass(unsigned int _width, unsigned int _height)
	:m_width(_width),
	m_height(_height)
{
	m_prepareShader = ShaderProgram::createShaderProgram("Resources/Shaders/DepthOfField/dofCombinedPrepare.comp");

	m_uFocalLength.create(m_prepareShader);
	m_uApertureSize.create(m_prepareShader);
	m_uNearFar.create(m_prepareShader);
}

void CombinedDofPrepareComputePass::execute(GLuint _colorTexture, GLuint _depthTexture, GLuint _tileMaxTexture, GLuint _destinationColorTexture, GLuint _destinationPresortTexture, float _fieldOfView, float _nearPlane, float _farPlane)
{
	m_prepareShader->bind();

	const float filmWidth = 0.035f;
	const float apertureSize = 8.0f;

	float focalLength = (0.5f * filmWidth) / glm::tan(_fieldOfView * 0.5f);

	m_uFocalLength.set(focalLength);
	m_uApertureSize.set(apertureSize);
	m_uNearFar.set(glm::vec2(_nearPlane, _farPlane));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _colorTexture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _depthTexture);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, _tileMaxTexture);

	glBindImageTexture(0, _destinationColorTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R11F_G11F_B10F);
	glBindImageTexture(1, _destinationPresortTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R11F_G11F_B10F);
	GLUtility::glDispatchComputeHelper(m_width / 2, m_height / 2, 1, 8, 8, 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

void CombinedDofPrepareComputePass::resize(unsigned int _width, unsigned int _height)
{
	m_width = _width;
	m_height = _height;
}
