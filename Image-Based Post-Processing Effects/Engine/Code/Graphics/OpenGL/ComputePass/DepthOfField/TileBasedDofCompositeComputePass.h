#pragma once
#include "Graphics\OpenGL\ShaderProgram.h"

struct Effects;
struct RenderData;

class TileBasedDofCompositeComputePass
{
public:
	explicit TileBasedDofCompositeComputePass(unsigned int _width, unsigned int _height);
	void execute(GLuint _colorTexture, GLuint _cocTexture, GLuint _destinationTexture);
	void resize(unsigned int _width, unsigned int _height);

private:
	std::shared_ptr<ShaderProgram> m_compositeShader;

	unsigned int m_width;
	unsigned int m_height;
};
