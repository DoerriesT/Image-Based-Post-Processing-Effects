#pragma once
#include "Graphics\OpenGL\ShaderProgram.h"

struct Effects;
struct RenderData;

class TileBasedDofFillComputePass
{
public:
	explicit TileBasedDofFillComputePass(unsigned int _width, unsigned int _height);
	void execute(GLuint *_dofTextures);
	void resize(unsigned int _width, unsigned int _height);

private:
	std::shared_ptr<ShaderProgram> m_fillShader;

	std::vector<GLint> m_uSampleCoords;

	bool m_fillSamplesSet;
	unsigned int m_width;
	unsigned int m_height;
};
