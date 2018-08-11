#pragma once
#include <glad/glad.h>

struct GBuffer
{
	GLuint albedoTexture;
	GLuint normalTexture;
	GLuint materialTexture;
	GLuint lightTextures[2];
	GLuint velocityTexture;
	GLuint ssaoTexture;
	GLuint depthStencilTexture;
};
