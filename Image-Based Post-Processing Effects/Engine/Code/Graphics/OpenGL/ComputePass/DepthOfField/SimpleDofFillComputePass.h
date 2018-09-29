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
	std::shared_ptr<ShaderProgram> fillShader;

	std::vector<GLint> uSampleCoordsDOFF;

	bool fillSamplesSet;
	unsigned int width;
	unsigned int height;
};