#pragma once
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"

struct Effects;

class AnamorphicUpsampleComputePass
{
public:
	explicit AnamorphicUpsampleComputePass(unsigned int _width, unsigned int _height);
	void execute(const Effects &_effects, GLuint _prefilterTexture, GLuint *_anamorphicTextureChain, size_t _chainSize, size_t _lastUsedTexture, unsigned int _lastWidth);
	void resize(unsigned int _width, unsigned int _height);

private:
	std::shared_ptr<ShaderProgram> m_upsampleShader;
	unsigned int m_width;
	unsigned int m_height;
};