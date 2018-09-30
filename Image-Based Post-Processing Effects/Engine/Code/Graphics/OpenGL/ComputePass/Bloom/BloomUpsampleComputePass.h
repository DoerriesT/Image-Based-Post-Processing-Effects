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
	std::shared_ptr<ShaderProgram> upsampleShader;

	Uniform<GLboolean> uAddPrevious = Uniform<GLboolean>("uAddPrevious");
	Uniform<GLfloat> uRadius = Uniform<GLfloat>("uRadius");
	Uniform<GLint> uLevel = Uniform<GLint>("uLevel");

	unsigned int width;
	unsigned int height;
};