#pragma once
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"

struct Effects;
struct RenderData;

class SimpleDofCocBlurComputePass
{
public:
	explicit SimpleDofCocBlurComputePass(unsigned int _width, unsigned int _height);
	void execute(GLuint _cocTexture, GLuint *_destinationTextures);
	void resize(unsigned int _width, unsigned int _height);

private:
	std::shared_ptr<ShaderProgram> cocBlurShader;

	Uniform<GLboolean> uDirectionCOCB = Uniform<GLboolean>("uDirection");

	unsigned int width;
	unsigned int height;
};