#pragma once
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"

struct Effects;
struct RenderData;

class SimpleDofCompositeComputePass
{
public:
	explicit SimpleDofCompositeComputePass(unsigned int _width, unsigned int _height);
	void execute(GLuint _destinationTexture);
	void resize(unsigned int _width, unsigned int _height);

private:
	std::shared_ptr<ShaderProgram> m_compositeShader;

	unsigned int m_width;
	unsigned int m_height;
};