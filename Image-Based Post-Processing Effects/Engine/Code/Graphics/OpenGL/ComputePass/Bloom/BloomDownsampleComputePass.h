#pragma once
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"

struct Effects;

class BloomDownsampleComputePass
{
public:
	explicit BloomDownsampleComputePass(unsigned int _width, unsigned int _height);
	void execute(GLuint _colorTexture, GLuint _destinationTexture);
	void resize(unsigned int _width, unsigned int _height);

private:
	std::shared_ptr<ShaderProgram> downsampleShader;

	Uniform<GLint> uLevel = Uniform<GLint>("uLevel");

	unsigned int width;
	unsigned int height;
};