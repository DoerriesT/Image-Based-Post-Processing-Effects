#include "VelocityCorrectionComputePass.h"
#include "Graphics\OpenGL\RenderData.h"
#include "Engine.h"
#include "Graphics\OpenGL\GLUtility.h"
#include "Graphics\OpenGL\GLTimerQuery.h"

static const char *CONSTANT_VELOCITY = "CONSTANT_VELOCITY";

VelocityCorrectionComputePass::VelocityCorrectionComputePass(unsigned int _width, unsigned int _height)
	:m_width(_width),
	m_height(_height)
{
	m_velocityCorrectionShader = ShaderProgram::createShaderProgram("Resources/Shaders/MotionBlur/correctVelocities.comp");

	m_uReprojection.create(m_velocityCorrectionShader);
	m_uScale.create(m_velocityCorrectionShader);
}

double velocityCorrectionComputeTime;
bool constantVelocity = false;

void VelocityCorrectionComputePass::execute(const RenderData & _renderData, GLuint _velocityTexture, GLuint _depthTexture)
{
	SCOPED_TIMER_QUERY(velocityCorrectionComputeTime);

	// shader permutations
	{
		const auto curDefines = m_velocityCorrectionShader->getDefines();

		bool constVel = false;

		for (const auto &define : curDefines)
		{
			if (std::get<0>(define) == ShaderProgram::ShaderType::COMPUTE)
			{
				if (std::get<1>(define) == CONSTANT_VELOCITY)
				{
					constVel = true;
				}
			}
		}

		if (constVel != constantVelocity)
		{
			m_velocityCorrectionShader->setDefines({ { ShaderProgram::ShaderType::COMPUTE, CONSTANT_VELOCITY, constantVelocity } });
			m_uReprojection.create(m_velocityCorrectionShader);
			m_uScale.create(m_velocityCorrectionShader);
		}
	}

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
