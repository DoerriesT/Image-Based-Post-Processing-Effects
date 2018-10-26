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
	std::shared_ptr<ShaderProgram> m_tileMaxShader;

	Uniform<GLfloat> m_uFocalLength = Uniform<GLfloat>("uFocalLength");
	Uniform<GLfloat> m_uApertureSize = Uniform<GLfloat>("uApertureSize");
	Uniform<glm::vec2> m_uNearFar = Uniform<glm::vec2>("uNearFar");
	Uniform<GLboolean> m_uDirection = Uniform<GLboolean>("uDirection");

	unsigned int m_width;
	unsigned int m_height;
};