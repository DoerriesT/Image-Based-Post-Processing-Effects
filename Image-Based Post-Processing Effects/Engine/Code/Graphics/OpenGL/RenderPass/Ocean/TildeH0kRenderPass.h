#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct OceanParams;

class TildeH0kRenderPass : public RenderPass
{
public:
	explicit TildeH0kRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const OceanParams &_water, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> m_tildeH0kShader;
	std::shared_ptr<Mesh> m_fullscreenTriangle;

	Uniform<GLint> m_uSimulationResolution = Uniform<GLint>("uN");
	Uniform<GLint> m_uWorldSize = Uniform<GLint>("uL");
	Uniform<GLfloat> m_uWaveAmplitude = Uniform<GLfloat>("uA");
	Uniform<glm::vec2> m_uWindDirection = Uniform<glm::vec2>("uWindDirection");
	Uniform<GLfloat> m_uWindSpeed = Uniform<GLfloat>("uWindSpeed");
	Uniform<GLfloat> m_uWaveSuppressionExp = Uniform<GLfloat>("uWaveSuppressionExp");

};