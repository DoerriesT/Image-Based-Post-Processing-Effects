#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"

struct Volume;

class GeometryInjectionRenderPass : public RenderPass
{
public:
	explicit GeometryInjectionRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const Volume &_geometryVolume,
		const glm::mat4 &_invViewProjection,
		GLint _normalTexture,
		const glm::vec3 &_lightDir,
		RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> geometryInjectionShader;
	GLuint VAO;
	GLuint VBO;

	Uniform<glm::mat4> uInvViewProjection = Uniform<glm::mat4>("uInvViewProjection");
	Uniform<GLint> uRsmWidth = Uniform<GLint>("uRsmWidth");
	Uniform<glm::vec3> uGridOrigin = Uniform<glm::vec3>("uGridOrigin");
	Uniform<glm::vec3> uGridSize = Uniform<glm::vec3>("uGridSize");
	Uniform<glm::vec2> uGridSpacing = Uniform<glm::vec2>("uGridSpacing"); // spacing, 1.0 / spacing
	Uniform<glm::vec3> uLightDirection = Uniform<glm::vec3>("uLightDirection");

};
