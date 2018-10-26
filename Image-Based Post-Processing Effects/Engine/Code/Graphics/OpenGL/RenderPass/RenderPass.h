#pragma once
#include <glad/glad.h>
#include <vector>

struct DepthState
{
	bool m_enabled;
	GLenum m_mask;
	GLenum m_func;
};

struct StencilState
{
	bool m_enabled;
	GLenum m_frontFunc;
	GLint m_frontRef;
	GLint m_frontMask;
	GLenum m_backFunc;
	GLint m_backRef;
	GLint m_backMask;
	GLenum m_frontOpFail;
	GLenum m_frontOpZfail;
	GLenum m_frontOpZpass;
	GLenum m_backOpFail;
	GLenum m_backOpZfail;
	GLenum m_backOpZpass;
};

struct CullFaceState
{
	bool m_enabled;
	GLenum m_face;
};

struct BlendState
{
	bool m_enabled;
	GLenum m_sFactor;
	GLenum m_dFactor;
};

struct ViewportState
{
	GLint m_x;
	GLint m_y;
	GLsizei m_width;
	GLsizei m_height;
};

struct State
{
	DepthState m_depthState;
	StencilState m_stencilState;
	CullFaceState m_cullFaceState;
	BlendState m_blendState;
	ViewportState m_viewportState;
};



class RenderPass
{
public:
	virtual void begin(const RenderPass *_previousRenderPass = nullptr);
	virtual void resize(unsigned int _width, unsigned int _height);

protected:
	GLuint m_fbo;
	std::vector<GLenum> m_drawBuffers;
	State m_state;

};