#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct LightPropagationVolume;

class LightInjectionRenderPass : public RenderPass
{
public:
	explicit LightInjectionRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const LightPropagationVolume &_lightPropagationVolume, const glm::mat4 &_invViewProjection, GLint _depthTexture, GLint _fluxTexture, GLint _normalTexture, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> lightInjectionShader;
	std::shared_ptr<Mesh> fullscreenTriangle;

	Uniform<glm::mat4> uInvViewProjection = Uniform<glm::mat4>("uInvViewProjection");
	Uniform<GLint> uRsmWidth = Uniform<GLint>("uRsmWidth");
	Uniform<glm::vec3> uGridOrigin = Uniform<glm::vec3>("uGridOrigin");
	Uniform<glm::vec3> uGridSize = Uniform<glm::vec3>("uGridSize");
	Uniform<glm::vec2> uGridSpacing = Uniform<glm::vec2>("uGridSpacing"); // spacing, 1.0 / spacing
	
};
