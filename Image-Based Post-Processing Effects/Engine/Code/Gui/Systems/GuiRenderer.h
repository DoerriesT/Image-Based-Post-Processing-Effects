#pragma once

#include "..\nuklearInclude.h"
#include <glad\glad.h>
#include "..\..\Framework\Graphics\Uniform.h"
#include "..\..\Settings.h"

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

class RenderSystem;
class ShaderProgram;

struct nk_glfw_device
{
	struct nk_buffer cmds;
	struct nk_draw_null_texture null;
	GLuint vbo, vao, ebo;
	GLint uniform_tex;
	GLint uniform_blur_tex;
	GLint uniform_blur_on;
	GLint uniform_proj;
	GLuint font_tex;
};

struct nk_glfw_vertex
{
	float position[2];
	float uv[2];
	nk_byte col[4];
};

class GuiRenderer
{
public:
	GuiRenderer();
	~GuiRenderer();

	void init();
	void render(nk_context *ctx);
	void uploadFontAtlas(const void *image, int width, int height);
	void fontStashBegin(struct nk_font_atlas *atlas);
	void fontStashEnd(struct nk_font_atlas *atlas);
	void setDimensions(float w, float h);

private:
	nk_glfw_device ogl_device;

	std::shared_ptr<ShaderProgram> guiShader;
	Uniform<GLint> uTexture = Uniform<GLint>("uTexture");
	Uniform<GLint> uBlurTexture = Uniform<GLint>("uBlurTexture");
	Uniform<GLint> uBlurOn = Uniform<GLint>("uBlurOn");
	Uniform<glm::mat4> uProjection = Uniform<glm::mat4>("uProjection");

	RenderSystem *renderSystem;

	glm::mat4 orthoProj;

	bool nvidia;
	void *vertexData;
	void *elementData;
	

	float width = 800;
	float height = 600;

	std::shared_ptr<Setting<bool>> guiBlur;
};