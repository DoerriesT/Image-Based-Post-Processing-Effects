#pragma once
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"

struct Effects;

class LuminanceHistogramAdaptionComputePass
{
public:
	explicit LuminanceHistogramAdaptionComputePass(unsigned int _width, unsigned int _height);
	void execute(GLuint _histogramTexture, GLuint *_temporalLuminanceTextures, bool _currentLuminanceTexture, const glm::vec2 &_params);
	void resize(unsigned int _width, unsigned int _height);

private:
	std::shared_ptr<ShaderProgram> m_adaptionShader;

	Uniform<GLfloat> m_uTimeDelta = Uniform<GLfloat>("uTimeDelta");
	Uniform<GLfloat> m_uTau = Uniform<GLfloat>("uTau");
	Uniform<glm::vec2> m_uParams = Uniform<glm::vec2>("uParams"); // multiply / add

	unsigned int m_width;
	unsigned int m_height;
};