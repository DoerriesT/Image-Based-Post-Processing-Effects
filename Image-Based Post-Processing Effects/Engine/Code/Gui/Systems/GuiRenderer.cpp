#include "GuiRenderer.h"
#include <stdint.h>
#include <assert.h>
#include <iostream>
#include <algorithm>
#include "Utilities\Utility.h"
#include "Settings.h"
#include "EntityComponentSystem\SystemManager.h"
#include "EntityComponentSystem\Systems\RenderSystem.h"
#include "Graphics\OpenGL\ShaderProgram.h"

GuiRenderer::GuiRenderer()
{
	orthoProj = glm::mat4(
		2.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -2.0f, 0.0f, 0.0f,
		0.0f, 0.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f, 1.0f);
	//orthoProj = glm::transpose(orthoProj);
}

GuiRenderer::~GuiRenderer()
{
	struct nk_glfw_device *dev = &ogl_device;
	glDeleteTextures(1, &dev->font_tex);
	glDeleteBuffers(1, &dev->vbo);
	glDeleteBuffers(1, &dev->ebo);
	nk_buffer_free(&dev->cmds);
	if (!nvidia)
	{
		free(vertexData);
		free(elementData);
	}
}

void GuiRenderer::init()
{
	std::string vendorString = (char *)glGetString(GL_VENDOR);
	std::transform(vendorString.begin(), vendorString.end(), vendorString.begin(), ::tolower);
	nvidia = vendorString.find("nvidia") != std::string::npos;

	guiShader = ShaderProgram::createShaderProgram("Resources/Shaders/Gui/gui.vert", "Resources/Shaders/Gui/gui.frag");

	// create uniforms
	uBlurOn.create(guiShader);
	uProjection.create(guiShader);

	struct nk_glfw_device *dev = &ogl_device;
	nk_buffer_init_default(&dev->cmds);

	//dev->uniform_blur_color = guiShader->createUniform("uBlurColor");

	guiBlur = SettingsManager::getInstance().getBoolSetting("graphics", "gui_blur_enabled", false);

	// vbo, vao, ebo setup
	GLsizei vs = sizeof(struct nk_glfw_vertex);
	size_t vp = offsetof(struct nk_glfw_vertex, position);
	size_t vt = offsetof(struct nk_glfw_vertex, uv);
	size_t vc = offsetof(struct nk_glfw_vertex, col);

	glGenBuffers(1, &dev->vbo);
	glGenBuffers(1, &dev->ebo);
	glGenVertexArrays(1, &dev->vao);

	glBindVertexArray(dev->vao);
	glBindBuffer(GL_ARRAY_BUFFER, dev->vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dev->ebo);

	if (!nvidia)
	{
		glBufferData(GL_ARRAY_BUFFER, MAX_VERTEX_BUFFER, NULL, GL_DYNAMIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_ELEMENT_BUFFER, NULL, GL_DYNAMIC_DRAW);

		vertexData = malloc(MAX_VERTEX_BUFFER);
		elementData = malloc(MAX_ELEMENT_BUFFER);
	}


	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, vs, (void*)vp);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, vs, (void*)vt);
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, vs, (void*)vc);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	renderSystem = SystemManager::getInstance().getSystem<RenderSystem>();
}


void GuiRenderer::render(nk_context *ctx)
{
	if (!ctx)
	{
		return;
	}

	const int max_vertex_buffer = MAX_VERTEX_BUFFER;
	const int max_element_buffer = MAX_ELEMENT_BUFFER;
	const nk_anti_aliasing AA = NK_ANTI_ALIASING_ON;

	struct nk_glfw_device *dev = &ogl_device;
	struct nk_buffer vbuf, ebuf;

	/* setup global state */
	glEnable(GL_BLEND);
	glEnable(GL_SCISSOR_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	glBlendEquation(GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE);

	
	const bool blurOn = guiBlur->get();

	/* setup program */
	guiShader->bind();
	uProjection.set(orthoProj);

	if (blurOn)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, renderSystem->getFinishedFrameTexture());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	glActiveTexture(GL_TEXTURE0);
	

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	{
		/* convert from command queue into draw list and draw to screen */
		const struct nk_draw_command *cmd;
		const nk_draw_index *offset = NULL;

		/* allocate vertex and element buffer */
		glBindVertexArray(dev->vao);
		glBindBuffer(GL_ARRAY_BUFFER, dev->vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dev->ebo);
		
		

		/* load draw vertices & elements directly into vertex + element buffer */
		if (nvidia)
		{
			glBufferData(GL_ARRAY_BUFFER, max_vertex_buffer, NULL, GL_STREAM_DRAW);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, max_element_buffer, NULL, GL_STREAM_DRAW);
			vertexData = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
			elementData = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
		}
		{
			/* fill convert configuration */
			struct nk_convert_config config;
			//memset(&config, 0, sizeof(config));

			static const struct nk_draw_vertex_layout_element vertex_layout[] = {
				{ NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_glfw_vertex, position) },
				{ NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_glfw_vertex, uv) },
				{ NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct nk_glfw_vertex, col) },
				{ NK_VERTEX_LAYOUT_END }
			};

			config.vertex_layout = vertex_layout;
			config.vertex_size = sizeof(struct nk_glfw_vertex);
			config.vertex_alignment = NK_ALIGNOF(struct nk_glfw_vertex);
			config.null = dev->null;
			config.circle_segment_count = 22;
			config.curve_segment_count = 22;
			config.arc_segment_count = 22;
			config.global_alpha = 1.0f;
			config.shape_AA = AA;
			config.line_AA = AA;

			/* setup buffers to load vertices and elements */
			nk_buffer_init_fixed(&vbuf, vertexData, (size_t)max_vertex_buffer);
			nk_buffer_init_fixed(&ebuf, elementData, (size_t)max_element_buffer);
			nk_convert(ctx, &dev->cmds, &vbuf, &ebuf, &config);
		}
		if (nvidia)
		{
			glUnmapBuffer(GL_ARRAY_BUFFER);
			glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
		}
		else
		{
			glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_VERTEX_BUFFER, vertexData);
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, MAX_ELEMENT_BUFFER, elementData);
		}

		/* iterate over and execute each draw command */
		nk_draw_foreach(cmd, ctx, &dev->cmds)
		{
			if (!cmd->elem_count) continue;

			bool blur = blurOn && cmd->clip_rect.x < -128 && cmd->clip_rect.y < -128;
			uBlurOn.set(blur);

			glBindTexture(GL_TEXTURE_2D, (GLuint)cmd->texture.id);
			glScissor(
				(GLint)(cmd->clip_rect.x),
				(GLint)(height - (GLint)(cmd->clip_rect.y + cmd->clip_rect.h)),
				(GLint)(cmd->clip_rect.w),
				(GLint)(cmd->clip_rect.h));
			glDrawElements(GL_TRIANGLES, (GLsizei)cmd->elem_count, GL_UNSIGNED_SHORT, offset);
			offset += cmd->elem_count;
		}
		nk_clear(ctx);
	}

	/* default OpenGL state */
	glDisable(GL_BLEND);
	glDisable(GL_SCISSOR_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	if (blurOn)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, renderSystem->getFinishedFrameTexture());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
}


void GuiRenderer::uploadFontAtlas(const void *image, int width, int height)
{
	struct nk_glfw_device *dev = &ogl_device;
	glGenTextures(1, &dev->font_tex);
	glBindTexture(GL_TEXTURE_2D, dev->font_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0,
				 GL_RGBA, GL_UNSIGNED_BYTE, image);
}


void GuiRenderer::fontStashBegin(struct nk_font_atlas *atlas)
{
	nk_font_atlas_init_default(atlas);
	nk_font_atlas_begin(atlas);
}

void GuiRenderer::fontStashEnd(struct nk_font_atlas *atlas)
{
	const void *image; int w, h;
	image = nk_font_atlas_bake(atlas, &w, &h, NK_FONT_ATLAS_RGBA32);

	uploadFontAtlas(image, w, h);

	nk_font_atlas_end(atlas, nk_handle_id((int)ogl_device.font_tex), &ogl_device.null);

}

void GuiRenderer::setDimensions(float w, float h)
{
	width = w;
	height = h;

	orthoProj[0][0] = 2.0f / width;
	orthoProj[1][1] = -2.0f / height;
}
