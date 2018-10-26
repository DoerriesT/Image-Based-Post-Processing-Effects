#pragma once
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"

class SeperateDofTileMaxComputePass
{
public:
	explicit SeperateDofTileMaxComputePass(unsigned int _width, unsigned int _height);
	void execute(GLuint _cocTexture);
	void resize(unsigned int _width, unsigned int _height);

private:
	std::shared_ptr<ShaderProgram> m_tileMaxShader;

	Uniform<GLint> m_uLevel = Uniform<GLint>("uLevel");

	unsigned int m_width;
	unsigned int m_height;
};
