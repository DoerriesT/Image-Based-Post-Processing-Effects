#include "GodRayGenComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"

GodRayGenComputePass::GodRayGenComputePass(unsigned int _width, unsigned int _height)
	:m_width(_width),
	m_height(_height)
{
	m_godRayGenShader = ShaderProgram::createShaderProgram("Resources/Shaders/GodRays/godRayGen.comp");

	m_uSunPos.create(m_godRayGenShader);
}

void GodRayGenComputePass::execute(const Effects & _effects, GLuint *_godRayTextures, const glm::vec3 &_lightPosition)
{
	m_godRayGenShader->bind();
	m_uSunPos.set(_lightPosition);

	for (unsigned int i = 0; i < 3; ++i)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _godRayTextures[i % 2]);

		glBindImageTexture(0, _godRayTextures[(i + 1) % 2], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		GLUtility::glDispatchComputeHelper(m_width / 2, m_height / 2, 1, 8, 8, 1);
		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	}
}

void GodRayGenComputePass::resize(unsigned int _width, unsigned int _height)
{
	m_width = _width;
	m_height = _height;
}
