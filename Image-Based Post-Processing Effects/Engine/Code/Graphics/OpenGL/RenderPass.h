#pragma once
#include<glad/glad.h>

struct DepthState
{
	bool enabled;
	GLenum mask;
	GLenum func;
};

struct StencilState
{
	bool enabled;
	GLenum func;
	GLint ref;
	GLint mask;
	GLenum opFail;
	GLenum opZfail;
	GLenum opZpass;
};

struct CullFaceState
{
	bool enabled;
	GLenum face;
};

struct State
{
	DepthState depthState;
	StencilState stencilState;
	CullFaceState cullFaceState;
};

class RenderPass
{
private:
	State startState;
};