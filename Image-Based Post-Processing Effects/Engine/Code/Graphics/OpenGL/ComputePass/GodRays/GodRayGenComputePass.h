#pragma once
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"

struct Effects;

class GodRayGenComputePass
{
public:
	explicit GodRayGenComputePass(unsigned int _width, unsigned int _height);
	void execute(const Effects &_effects, GLuint *_godRayTextures, const glm::vec3 &_lightPosition);
	void resize(unsigned int _width, unsigned int _height);

private:
	std::shared_ptr<ShaderProgram> m_godRayGenShader;

	Uniform<glm::vec3> m_uSunPos = Uniform<glm::vec3>("uSunPos");

	unsigned int m_width;
	unsigned int m_height;
};