#pragma once
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"

struct Effects;

class AnamorphicPrefilterComputePass
{
public:
	explicit AnamorphicPrefilterComputePass(unsigned int _width, unsigned int _height);
	void execute(const Effects &_effects, GLuint _sourceTexture, GLuint _prefilterTexture);
	void resize(unsigned int _width, unsigned int _height);

private:
	std::shared_ptr<ShaderProgram> prefilterShader;
	unsigned int width;
	unsigned int height;
};