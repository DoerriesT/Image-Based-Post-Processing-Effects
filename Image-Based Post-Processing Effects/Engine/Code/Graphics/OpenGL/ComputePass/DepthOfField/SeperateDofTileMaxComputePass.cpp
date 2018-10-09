#include "SeperateDofTileMaxComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"

SeperateDofTileMaxComputePass::SeperateDofTileMaxComputePass(unsigned int _width, unsigned int _height)
	:width(_width),
	height(_height)
{
	tileMaxShader = ShaderProgram::createShaderProgram("Resources/Shaders/DepthOfField/dofSeperatedTileMax.comp");

	uLevel.create(tileMaxShader);
}

void SeperateDofTileMaxComputePass::execute(GLuint _cocTexture)
{
	tileMaxShader->bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _cocTexture);

	unsigned int w = width / 2;
	unsigned int h = height / 2;

	for (unsigned int i = 0; i < 5; ++i)
	{
		w /= 2;
		h /= 2;

		uLevel.set(static_cast<GLint>(i));

		glBindImageTexture(0, _cocTexture, i + 1, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
		GLUtility::glDispatchComputeHelper(w, h, 1, 8, 8, 1);
		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	}
}

void SeperateDofTileMaxComputePass::resize(unsigned int _width, unsigned int _height)
{
	width = _width;
	height = _height;
}
