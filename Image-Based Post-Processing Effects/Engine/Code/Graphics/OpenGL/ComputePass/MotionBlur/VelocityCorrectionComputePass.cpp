#include "VelocityCorrectionComputePass.h"
#include "Graphics\OpenGL\RenderData.h"
#include "Engine.h"
#include "Graphics\OpenGL\GLUtility.h"

VelocityCorrectionComputePass::VelocityCorrectionComputePass(unsigned int _width, unsigned int _height)
	:m_width(_width),
	m_height(_height)
{
	m_velocityCorrectionShader = ShaderProgram::createShaderProgram("Resources/Shaders/MotionBlur/correctVelocities.comp");

	m_uReprojection.create(m_velocityCorrectionShader);
	m_uScale.create(m_velocityCorrectionShader);
}

void VelocityCorrectionComputePass::execute(const RenderData & _renderData, GLuint _velocityTexture, GLuint _depthTexture)
{
	m_velocityCorrectionShader->bind();

	m_uReprojection.set(_renderData.m_prevViewProjectionMatrix * _renderData.m_invViewProjectionMatrix);
	constexpr float targetFrametime = 1.0f / 60.0f;
	const float scaleCorrection = targetFrametime / (float)Engine::getTimeDelta();
	m_uScale.set(scaleCorrection);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _depthTexture);

	glBindImageTexture(0, _velocityTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG16F);
	GLUtility::glDispatchComputeHelper(m_width, m_height, 1, 8, 8, 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

void VelocityCorrectionComputePass::resize(unsigned int _width, unsigned int _height)
{
	m_width = _width;
	m_height = _height;
}
