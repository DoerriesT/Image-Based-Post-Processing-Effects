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
	std::shared_ptr<ShaderProgram> m_spriteShader;

	GLuint m_spriteVAO;
	GLuint m_spriteVBO;
	GLuint m_spriteEBO;

	Uniform<GLint> m_uWidth = Uniform<GLint>("uWidth");
	Uniform<GLint> m_uHeight = Uniform<GLint>("uHeight");

};
