#pragma once
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"

class CombinedDofPrepareComputePass
{
public:
	explicit CombinedDofPrepareComputePass(unsigned int _width, unsigned int _height);
	void execute(GLuint _colorTexture, GLuint _depthTexture, GLuint _tileMaxTexture, GLuint _destinationColorTexture, GLuint _destinationPresortTexture, float _fieldOfView, float _nearPlane, float _farPlane);
	void resize(unsigned int _width, unsigned int _height);

private:
	std::shared_ptr<ShaderProgram> prepareShader;

	Uniform<GLfloat> uFocalLength = Uniform<GLfloat>("uFocalLength");
	Uniform<GLfloat> uApertureSize = Uniform<GLfloat>("uApertureSize");
	Uniform<glm::vec2> uNearFar = Uniform<glm::vec2>("uNearFar");

	unsigned int width;
	unsigned int height;
};