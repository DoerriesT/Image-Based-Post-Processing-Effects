#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"

struct Volume;

class LightInjectionRenderPass : public RenderPass
{
public:
	explicit LightInjectionRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const Volume &_lightPropagationVolume, 
		const glm::mat4 &_invViewProjection, 
		GLint _depthTexture, 
		GLint _fluxTexture, 
		GLint _normalTexture,
		RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> m_lightInjectionShader;
	GLuint m_VAO;
	GLuint m_VBO;

	Uniform<glm::mat4> m_uInvViewProjection = Uniform<glm::mat4>("uInvViewProjection");
	Uniform<GLint> m_uRsmWidth = Uniform<GLint>("uRsmWidth");
	Uniform<glm::vec3> m_uGridOrigin = Uniform<glm::vec3>("uGridOrigin");
	Uniform<glm::vec3> m_uGridSize = Uniform<glm::vec3>("uGridSize");
	Uniform<glm::vec2> m_uGridSpacing = Uniform<glm::vec2>("uGridSpacing"); // spacing, 1.0 / spacing
	
};
