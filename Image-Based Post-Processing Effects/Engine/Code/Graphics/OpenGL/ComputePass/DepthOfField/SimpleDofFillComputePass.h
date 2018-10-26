#pragma once
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"

struct Effects;
struct RenderData;

class SimpleDofFillComputePass
{
public:
	explicit SimpleDofFillComputePass(unsigned int _width, unsigned int _height);
	void execute(GLuint *_resultTextures);
	void resize(unsigned int _width, unsigned int _height);

private:
	std::shared_ptr<ShaderProgram> m_fillShader;

	std::vector<GLint> m_uSampleCoordsDOFF;

	bool m_fillSamplesSet;
	unsigned int m_width;
	unsigned int m_height;
};