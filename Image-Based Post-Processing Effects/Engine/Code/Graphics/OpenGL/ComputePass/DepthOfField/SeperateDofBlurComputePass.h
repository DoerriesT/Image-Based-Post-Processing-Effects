#pragma once
#include "Graphics\OpenGL\ShaderProgram.h"

struct Effects;
struct RenderData;

class SeperateDofBlurComputePass
{
public:
	explicit SeperateDofBlurComputePass(unsigned int _width, unsigned int _height);
	void execute(GLuint *_dofTextures, GLuint _cocTexture, GLuint _cocTileTexture);
	void resize(unsigned int _width, unsigned int _height);

private:
	std::shared_ptr<ShaderProgram> m_blurShader;

	std::vector<GLint> m_uSampleCoords;

	bool m_blurSamplesSet;
	unsigned int m_width;
	unsigned int m_height;
};
