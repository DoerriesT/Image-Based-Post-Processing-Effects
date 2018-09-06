#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct LightPropagationVolume;

class LightPropagationRenderPass : public RenderPass
{
public:
	explicit LightPropagationRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const LightPropagationVolume &_lightPropagationVolume, GLint _redTexture, GLint _greenTexture, GLint _blueTexture, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> lightPropagationShader;
	std::shared_ptr<Mesh> fullscreenTriangle;

	Uniform<glm::vec3> uGridSize = Uniform<glm::vec3>("uGridSize");

};
