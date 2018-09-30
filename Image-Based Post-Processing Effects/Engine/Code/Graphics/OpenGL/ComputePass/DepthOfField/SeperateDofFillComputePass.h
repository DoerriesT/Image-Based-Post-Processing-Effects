#pragma once
#include "Graphics\OpenGL\ShaderProgram.h"

struct Effects;
struct RenderData;

class SeperateDofFillComputePass
{
public:
	explicit SeperateDofFillComputePass(unsigned int _width, unsigned int _height);
	void execute(GLuint *_dofTextures);
	void resize(unsigned int _width, unsigned int _height);

private:
	std::shared_ptr<ShaderProgram> fillShader;

	std::vector<GLint> uSampleCoordsSDOFF;

	bool fillSamplesSet;
	unsigned int width;
	unsigned int height;
};
