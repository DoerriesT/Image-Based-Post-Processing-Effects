#pragma once
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"

struct Effects;

class LuminanceGenComputePass
{
public:
	explicit LuminanceGenComputePass(unsigned int _width, unsigned int _height);
	void execute(const Effects &_effects, GLuint _colorTexture, GLuint _luminanceTexture);
	void resize(unsigned int _width, unsigned int _height);

private:
	std::shared_ptr<ShaderProgram> m_luminanceGenShader;
	unsigned int m_width;
	unsigned int m_height;
};