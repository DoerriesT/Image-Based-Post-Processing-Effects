#include "VelocityCorrectionComputePass.h"
#include "Graphics\OpenGL\RenderData.h"
#include "Engine.h"
#include "Graphics\OpenGL\GLUtility.h"

VelocityCorrectionComputePass::VelocityCorrectionComputePass(unsigned int _width, unsigned int _height)
	:width(_width),
	height(_height)
{
	velocityCorrectionShader = ShaderProgram::createShaderProgram("Resources/Shaders/MotionBlur/correctVelocities.comp");

	uReprojectionVC.create(velocityCorrectionShader);
	uScaleVC.create(velocityCorrectionShader);
}

void VelocityCorrectionComputePass::execute(const RenderData & _renderData, GLuint _velocityTexture, GLuint _depthTexture)
{
	velocityCorrectionShader->bind();

	uReprojectionVC.set(_renderData.prevViewProjectionMatrix * _renderData.invViewProjectionMatrix);
	constexpr float targetFrametime = 1.0f / 60.0f;
	const float scaleCorrection = targetFrametime / (float)Engine::getTimeDelta();
	uScaleVC.set(scaleCorrection);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _depthTexture);

	glBindImageTexture(0, _velocityTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG16F);
	GLUtility::glDispatchComputeHelper(width, height, 1, 8, 8, 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

void VelocityCorrectionComputePass::resize(unsigned int _width, unsigned int _height)
{
	width = _width;
	height = _height;
}
