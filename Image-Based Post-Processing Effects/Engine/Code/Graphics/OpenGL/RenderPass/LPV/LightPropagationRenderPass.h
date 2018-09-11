#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"

struct Volume;

class LightPropagationRenderPass : public RenderPass
{
public:
	explicit LightPropagationRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const Volume &_lightPropagationVolume, GLint _geometryTexture, GLint *_redTexture, GLint *_greenTexture, GLint *_blueTexture, GLint *accumTextures, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> lightPropagationShader;
	GLuint VAO;
	GLuint VBO;

	Uniform<glm::vec3> uGridSize = Uniform<glm::vec3>("uGridSize");
	Uniform<GLboolean> uFirstIteration = Uniform<GLboolean>("uFirstIteration");

};
