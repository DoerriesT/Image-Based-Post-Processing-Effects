#pragma once
#include "Graphics\OpenGL\ShaderProgram.h"

struct Effects;
struct RenderData;

class SeperateDofDownsampleComputePass
{
public:
	explicit SeperateDofDownsampleComputePass(unsigned int _width, unsigned int _height);
	void execute(GLuint _colorTexture, GLuint _cocTexture, GLuint _destinationCocTexture, GLuint _destinationNearTexture, GLuint _destinationFarTexture);
	void resize(unsigned int _width, unsigned int _height);

private:
	std::shared_ptr<ShaderProgram> downsampleShader;

	unsigned int width;
	unsigned int height;
};