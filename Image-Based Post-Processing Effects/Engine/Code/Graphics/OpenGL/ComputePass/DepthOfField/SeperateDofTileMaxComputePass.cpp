#include "SeperateDofTileMaxComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"

SeperateDofTileMaxComputePass::SeperateDofTileMaxComputePass(unsigned int _width, unsigned int _height)
	:m_width(_width),
	m_height(_height)
{
	m_tileMaxShader = ShaderProgram::createShaderProgram("Resources/Shaders/DepthOfField/dofSeperatedTileMax.comp");

	m_uLevel.create(m_tileMaxShader);
}

void SeperateDofTileMaxComputePass::execute(GLuint _cocTexture)
{
	m_tileMaxShader->bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _cocTexture);

	unsigned int w = m_width / 2;
	unsigned int h = m_height / 2;

	for (unsigned int i = 0; i < 5; ++i)
	{
		w /= 2;
		h /= 2;

		m_uLevel.set(static_cast<GLint>(i));

		glBindImageTexture(0, _cocTexture, i + 1, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
		GLUtility::glDispatchComputeHelper(w, h, 1, 8, 8, 1);
		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	}
}

void SeperateDofTileMaxComputePass::resize(unsigned int _width, unsigned int _height)
{
	m_width = _width;
	m_height = _height;
}
