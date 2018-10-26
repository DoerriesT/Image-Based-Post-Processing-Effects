#pragma once
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"

struct Effects;

class LuminanceAdaptionComputePass
{
public:
	explicit LuminanceAdaptionComputePass(unsigned int _width, unsigned int _height);
	void execute(const Effects &_effects, GLuint _luminanceTexture, GLuint *_temporalLuminanceTextures, bool _currentLuminanceTexture);
	void resize(unsigned int _width, unsigned int _height);

private:
	std::shared_ptr<ShaderProgram> m_luminanceAdaptionShader;

	Uniform<GLfloat> m_uTimeDelta = Uniform<GLfloat>("uTimeDelta");
	Uniform<GLfloat> m_uTau = Uniform<GLfloat>("uTau");

	unsigned int m_width;
	unsigned int m_height;
};