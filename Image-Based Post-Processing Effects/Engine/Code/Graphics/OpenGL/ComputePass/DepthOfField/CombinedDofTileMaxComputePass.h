#pragma once
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"

class CombinedDofTileMaxComputePass
{
public:
	explicit CombinedDofTileMaxComputePass(unsigned int _width, unsigned int _height);
	void execute(GLuint _depthTexture, GLuint _tmpTexture, GLuint _tileMaxTexture, unsigned int _tileSize, float _fieldOfView, float _nearPlane, float _farPlane);
	void resize(unsigned int _width, unsigned int _height);

private:
	std::shared_ptr<ShaderProgram> tileMaxShader;

	Uniform<GLfloat> uFocalLength = Uniform<GLfloat>("uFocalLength");
	Uniform<GLfloat> uApertureSize = Uniform<GLfloat>("uApertureSize");
	Uniform<glm::vec2> uNearFar = Uniform<glm::vec2>("uNearFar");
	Uniform<GLboolean> uDirection = Uniform<GLboolean>("uDirection");

	unsigned int width;
	unsigned int height;
};