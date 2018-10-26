#include "CombinedDofNeighborTileMaxComputePass.h"
#include "Graphics\OpenGL\GLUtility.h"

CombinedDofNeighborTileMaxComputePass::CombinedDofNeighborTileMaxComputePass(unsigned int _width, unsigned int _height)
	:m_width(_width),
	m_height(_height)
{
	m_neighborTileMaxShader = ShaderProgram::createShaderProgram("Resources/Shaders/DepthOfField/dofCombinedCocNeighborTileMax.comp");
}

void CombinedDofNeighborTileMaxComputePass::execute(GLuint _tileMaxTexture, GLuint _neighborTileMaxTexture, unsigned int _tileSize)
{
	m_neighborTileMaxShader->bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _tileMaxTexture);

	glBindImageTexture(0, _neighborTileMaxTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
	GLUtility::glDispatchComputeHelper(m_width / _tileSize, m_height / _tileSize, 1, 8, 8, 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

}

void CombinedDofNeighborTileMaxComputePass::resize(unsigned int _width, unsigned int _height)
{
	m_width = _width;
	m_height = _height;
}
