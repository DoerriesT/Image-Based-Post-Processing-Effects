#pragma once
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"

struct Effects;

class GodRayMaskComputePass
{
public:
	explicit GodRayMaskComputePass(unsigned int _width, unsigned int _height);
	void execute(const Effects &_effects, GLuint _colorTexture, GLuint _depthTexture, GLuint _godRayTexture);
	void resize(unsigned int _width, unsigned int _height);

private:
	std::shared_ptr<ShaderProgram> godRayMaskShader;

	unsigned int width;
	unsigned int height;
};