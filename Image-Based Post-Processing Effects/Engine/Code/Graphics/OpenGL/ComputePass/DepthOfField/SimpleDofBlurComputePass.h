#pragma once
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"

struct Effects;
struct RenderData;

class SimpleDofBlurComputePass
{
public:
	explicit SimpleDofBlurComputePass(unsigned int _width, unsigned int _height);
	void execute(GLuint _colorTexture, GLuint _cocTexture, GLuint *_dofTextures);
	void resize(unsigned int _width, unsigned int _height);

private:
	std::shared_ptr<ShaderProgram> m_blurShader;

	std::vector<GLint> m_uSampleCoords;

	bool m_blurSamplesSet;
	unsigned int m_width;
	unsigned int m_height;
};