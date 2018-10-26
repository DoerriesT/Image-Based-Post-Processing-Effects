#pragma once
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"

class BloomUpsampleComputePass
{
public:
	explicit BloomUpsampleComputePass(unsigned int _width, unsigned int _height);
	void execute(GLuint _sourceTexture, GLuint _destinationTexture);
	void resize(unsigned int _width, unsigned int _height);

private:
	std::shared_ptr<ShaderProgram> m_upsampleShader;

	Uniform<GLboolean> m_uAddPrevious = Uniform<GLboolean>("uAddPrevious");
	Uniform<GLfloat> m_uRadius = Uniform<GLfloat>("uRadius");
	Uniform<GLint> m_uLevel = Uniform<GLint>("uLevel");

	unsigned int m_width;
	unsigned int m_height;
};