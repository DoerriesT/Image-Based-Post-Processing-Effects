#pragma once
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"

class LuminanceHistogramComputePass
{
public:
	explicit LuminanceHistogramComputePass(unsigned int _width, unsigned int _height);
	void execute(GLuint _colorTexture, GLuint _intermediaryTexture, const glm::vec2 &_params);
	void resize(unsigned int _width, unsigned int _height);

private:
	std::shared_ptr<ShaderProgram> m_histogramShader;

	Uniform<glm::vec2> m_uParams = Uniform<glm::vec2>("uParams"); // multiply / add

	unsigned int m_width;
	unsigned int m_height;
};