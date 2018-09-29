#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

class SpriteDofRenderPass : public RenderPass
{
public:
	explicit SpriteDofRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(GLuint _colorTexture, GLuint _depthTexture, GLuint _cocTexture, GLuint _destinationTexture, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> spriteShader;
	std::shared_ptr<Mesh> fullscreenTriangle;

	GLuint spriteVAO;
	GLuint spriteVBO;
	GLuint spriteEBO;

	Uniform<GLint> uWidthDOF = Uniform<GLint>("uWidth");
	Uniform<GLint> uHeightDOF = Uniform<GLint>("uHeight");

};
