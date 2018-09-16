#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct Volume;

class LightPropagationRenderPass : public RenderPass
{
public:
	explicit LightPropagationRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const Volume &_lightPropagationVolume, GLuint _geometryTexture, GLuint *_redTexture, GLuint *_greenTexture, GLuint *_blueTexture, GLuint *accumTextures, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> lightPropagationShader;
	std::shared_ptr<Mesh> fullscreenTriangle;

	Uniform<glm::vec3> uGridSize = Uniform<glm::vec3>("uGridSize");
	Uniform<GLboolean> uFirstIteration = Uniform<GLboolean>("uFirstIteration");
	Uniform<GLfloat> uOcclusionAmplifier = Uniform<GLfloat>("uOcclusionAmplifier");

};
