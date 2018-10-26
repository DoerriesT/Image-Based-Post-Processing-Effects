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
	void render(const Volume &_lightPropagationVolume, GLuint _geometryTexture, GLuint *_redTexture, GLuint *_greenTexture, GLuint *_blueTexture, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> m_lightPropagationShader;
	std::shared_ptr<Mesh> m_fullscreenTriangle;

	Uniform<glm::vec3> m_uGridSize = Uniform<glm::vec3>("uGridSize");
	Uniform<GLboolean> m_uFirstIteration = Uniform<GLboolean>("uFirstIteration");
	Uniform<GLfloat> m_uOcclusionAmplifier = Uniform<GLfloat>("uOcclusionAmplifier");

};
