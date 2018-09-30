#pragma once
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"

class LuminanceHistogramReduceComputePass
{
public:
	explicit LuminanceHistogramReduceComputePass(unsigned int _width, unsigned int _height);
	void execute(GLuint _histogramTexture);
	void resize(unsigned int _width, unsigned int _height);

private:
	std::shared_ptr<ShaderProgram> reduceShader;

	Uniform<GLint> uLinesLHR = Uniform<GLint>("uLines");

	unsigned int width;
	unsigned int height;
};