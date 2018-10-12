#pragma once
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"

struct Effects;
struct RenderData;

class AntiAliasingReverseTonemapComputePass
{
public:
	explicit AntiAliasingReverseTonemapComputePass(unsigned int _width, unsigned int _height);
	void execute(GLuint _colorTexture);
	void resize(unsigned int _width, unsigned int _height);

private:
	std::shared_ptr<ShaderProgram> reverseTonemapShader;

	unsigned int width;
	unsigned int height;
};