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
	std::shared_ptr<ShaderProgram> adaptionShader;

	Uniform<GLfloat> uTimeDeltaLHA = Uniform<GLfloat>("uTimeDelta");
	Uniform<GLfloat> uTauLHA = Uniform<GLfloat>("uTau");
	Uniform<glm::vec2> uParamsLHA = Uniform<glm::vec2>("uParams"); // multiply / add

	unsigned int width;
	unsigned int height;
};