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
	std::shared_ptr<ShaderProgram> m_cocShader;

	Uniform<GLfloat> m_uFocalLength = Uniform<GLfloat>("uFocalLength");
	Uniform<GLfloat> m_uApertureSize = Uniform<GLfloat>("uApertureSize");
	Uniform<glm::vec2> m_uNearFar = Uniform<glm::vec2>("uNearFar");

	unsigned int m_width;
	unsigned int m_height;
};
