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
	std::shared_ptr<ShaderProgram> luminanceAdaptionShader;

	Uniform<GLfloat> uTimeDeltaLA = Uniform<GLfloat>("uTimeDelta");
	Uniform<GLfloat> uTauLA = Uniform<GLfloat>("uTau");

	unsigned int width;
	unsigned int height;
};