#pragma once
#include "Graphics\OpenGL\ShaderProgram.h"

struct Effects;
struct RenderData;

class SeperateDofCompositeComputePass
{
public:
	explicit SeperateDofCompositeComputePass(unsigned int _width, unsigned int _height);
	void execute(GLuint _colorTexture, GLuint _cocTexture, GLuint _destinationTexture);
	void resize(unsigned int _width, unsigned int _height);

private:
	std::shared_ptr<ShaderProgram> compositeShader;

	unsigned int width;
	unsigned int height;
};
