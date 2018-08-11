#pragma once
#include <glad/glad.h>
#include <vector>

struct DepthState
{
	bool enabled;
	GLenum mask;
	GLenum func;
};

struct StencilState
{
	bool enabled;
	GLenum frontFunc;
	GLint frontRef;
	GLint frontMask;
	GLenum backFunc;
	GLint backRef;
	GLint backMask;
	GLenum frontOpFail;
	GLenum frontOpZfail;
	GLenum frontOpZpass;
	GLenum backOpFail;
	GLenum backOpZfail;
	GLenum backOpZpass;
};

struct CullFaceState
{
	bool enabled;
	GLenum face;
};

struct BlendState
{
	bool enabled;
	GLenum sFactor;
	GLenum dFactor;
};

struct ViewportState
{
	GLint x;
	GLint y;
	GLsizei width;
	GLsizei height;
};

struct State
{
	DepthState depthState;
	StencilState stencilState;
	CullFaceState cullFaceState;
	BlendState blendState;
	ViewportState viewportState;
};



class RenderPass
{
public:
	virtual void begin(const RenderPass *_previousRenderPass = nullptr);
	virtual void resize(unsigned int _width, unsigned int _height);

protected:
	GLuint fbo;
	std::vector<GLenum> drawBuffers;
	State state;

};