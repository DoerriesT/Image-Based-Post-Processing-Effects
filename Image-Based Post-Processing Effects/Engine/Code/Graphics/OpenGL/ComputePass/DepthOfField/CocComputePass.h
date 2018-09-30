#pragma once
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"

class CocComputePass
{
public:
	explicit CocComputePass(unsigned int _width, unsigned int _height);
	void execute(GLuint _depthTexture, GLuint _destinationCocTexture, float _fieldOfView, float _nearPlane, float _farPlane);
	void resize(unsigned int _width, unsigned int _height);

private:
	std::shared_ptr<ShaderProgram> cocShader;

	Uniform<GLfloat> uFocalLengthCOC = Uniform<GLfloat>("uFocalLength");
	Uniform<GLfloat> uApertureSizeCOC = Uniform<GLfloat>("uApertureSize");
	Uniform<glm::vec2> uNearFarCOC = Uniform<glm::vec2>("uNearFar");

	unsigned int width;
	unsigned int height;
};
