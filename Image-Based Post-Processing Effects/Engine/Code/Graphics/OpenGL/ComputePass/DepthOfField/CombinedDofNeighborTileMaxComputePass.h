#pragma once
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"

class CombinedDofNeighborTileMaxComputePass
{
public:
	explicit CombinedDofNeighborTileMaxComputePass(unsigned int _width, unsigned int _height);
	void execute(GLuint _tileMaxTexture, GLuint _neighborTileMaxTexture, unsigned int _tileSize);
	void resize(unsigned int _width, unsigned int _height);

private:
	std::shared_ptr<ShaderProgram> m_neighborTileMaxShader;

	unsigned int m_width;
	unsigned int m_height;
};