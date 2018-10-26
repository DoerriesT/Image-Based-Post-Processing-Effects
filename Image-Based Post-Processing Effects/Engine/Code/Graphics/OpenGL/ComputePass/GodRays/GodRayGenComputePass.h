#pragma once
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"

struct Effects;

class GodRayGenComputePass
{
public:
	explicit GodRayGenComputePass(unsigned int _width, unsigned int _height);
	void execute(const Effects &_effects, GLuint *_godRayTextures, const glm::vec2 &_lightPosition);
	void resize(unsigned int _width, unsigned int _height);

private:
	std::shared_ptr<ShaderProgram> m_godRayGenShader;

	Uniform<glm::vec2> m_uSunPos = Uniform<glm::vec2>("uSunPos");

	unsigned int m_width;
	unsigned int m_height;
};