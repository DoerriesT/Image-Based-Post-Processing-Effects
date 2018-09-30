#include "CocComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"

CocComputePass::CocComputePass(unsigned int _width, unsigned int _height)
	:width(_width),
	height(_height)
{
	cocShader = ShaderProgram::createShaderProgram("Resources/Shaders/DepthOfField/coc.comp");

	uFocalLengthCOC.create(cocShader);
	uApertureSizeCOC.create(cocShader);
	uNearFarCOC.create(cocShader);
}

void CocComputePass::execute(GLuint _depthTexture, GLuint _destinationCocTexture, float _fieldOfView, float _nearPlane, float _farPlane)
{
	cocShader->bind();

	const float filmWidth = 35.0f;
	const float apertureSize = 8.0f;

	float focalLength = (0.5f * filmWidth) / glm::tan(_fieldOfView * 0.5f);

	uFocalLengthCOC.set(focalLength);
	uApertureSizeCOC.set(8.0f);
	uNearFarCOC.set(glm::vec2(_nearPlane, _farPlane));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _depthTexture);

	glBindImageTexture(0, _destinationCocTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
	GLUtility::glDispatchComputeHelper(width, height, 1, 8, 8, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void CocComputePass::resize(unsigned int _width, unsigned int _height)
{
	width = _width;
	height = _height;
}
