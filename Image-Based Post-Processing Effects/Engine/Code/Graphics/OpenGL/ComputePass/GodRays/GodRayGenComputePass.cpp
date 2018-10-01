#include "GodRayGenComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"

GodRayGenComputePass::GodRayGenComputePass(unsigned int _width, unsigned int _height)
	:width(_width),
	height(_height)
{
	godRayGenShader = ShaderProgram::createShaderProgram("Resources/Shaders/GodRays/godRayGen.comp");

	uSunPosGR.create(godRayGenShader);
}

void GodRayGenComputePass::execute(const Effects & _effects, GLuint *_godRayTextures, glm::vec2 _lightPosition)
{
	godRayGenShader->bind();
	uSunPosGR.set(_lightPosition);

	for (unsigned int i = 0; i < 3; ++i)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _godRayTextures[i % 2]);

		glBindImageTexture(0, _godRayTextures[(i + 1) % 2], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		GLUtility::glDispatchComputeHelper(width / 2, height / 2, 1, 8, 8, 1);
		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	}
}

void GodRayGenComputePass::resize(unsigned int _width, unsigned int _height)
{
	width = _width;
	height = _height;
}
