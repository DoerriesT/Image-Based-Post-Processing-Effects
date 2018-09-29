#pragma once
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"

struct Effects;

class AnamorphicDownsampleComputePass
{
public:
	explicit AnamorphicDownsampleComputePass(unsigned int _width, unsigned int _height);
	void execute(const Effects &_effects, GLuint _prefilterTexture, GLuint *_anamorphicTextureChain, size_t _chainSize, int &_lastUsedTexture, unsigned int &_lastWidth);
	void resize(unsigned int _width, unsigned int _height);

private:
	std::shared_ptr<ShaderProgram> downsampleShader;
	unsigned int width;
	unsigned int height;
};