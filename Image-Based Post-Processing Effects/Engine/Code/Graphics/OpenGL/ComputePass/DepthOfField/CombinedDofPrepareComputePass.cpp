#include "CombinedDofPrepareComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"

CombinedDofPrepareComputePass::CombinedDofPrepareComputePass(unsigned int _width, unsigned int _height)
	:width(_width),
	height(_height)
{
	prepareShader = ShaderProgram::createShaderProgram("Resources/Shaders/DepthOfField/dofCombinedPrepare.comp");

	uFocalLength.create(prepareShader);
	uApertureSize.create(prepareShader);
	uNearFar.create(prepareShader);
}

void CombinedDofPrepareComputePass::execute(GLuint _colorTexture, GLuint _depthTexture, GLuint _tileMaxTexture, GLuint _destinationColorTexture, GLuint _destinationPresortTexture, float _fieldOfView, float _nearPlane, float _farPlane)
{
	prepareShader->bind();

	const float filmWidth = 0.035f;
	const float apertureSize = 8.0f;

	float focalLength = (0.5f * filmWidth) / glm::tan(_fieldOfView * 0.5f);

	uFocalLength.set(focalLength);
	uApertureSize.set(apertureSize);
	uNearFar.set(glm::vec2(_nearPlane, _farPlane));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _colorTexture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _depthTexture);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, _tileMaxTexture);

	glBindImageTexture(0, _destinationColorTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R11F_G11F_B10F);
	glBindImageTexture(1, _destinationPresortTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R11F_G11F_B10F);
	GLUtility::glDispatchComputeHelper(width / 2, height / 2, 1, 8, 8, 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

void CombinedDofPrepareComputePass::resize(unsigned int _width, unsigned int _height)
{
	width = _width;
	height = _height;
}
