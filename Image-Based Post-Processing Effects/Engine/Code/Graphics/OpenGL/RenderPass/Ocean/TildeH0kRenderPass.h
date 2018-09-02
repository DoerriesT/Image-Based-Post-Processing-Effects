#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct Water;

class TildeH0kRenderPass : public RenderPass
{
public:
	explicit TildeH0kRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const Water &_water, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> tildeH0kShader;
	std::shared_ptr<Mesh> fullscreenTriangle;

	Uniform<GLint> uSimulationResolutionH0 = Uniform<GLint>("uN");
	Uniform<GLint> uWorldSizeH0 = Uniform<GLint>("uL");
	Uniform<GLfloat> uWaveAmplitudeH0 = Uniform<GLfloat>("uA");
	Uniform<glm::vec2> uWindDirectionH0 = Uniform<glm::vec2>("uWindDirection");
	Uniform<GLfloat> uWindSpeedH0 = Uniform<GLfloat>("uWindSpeed");
	Uniform<GLfloat> uWaveSuppressionExpH0 = Uniform<GLfloat>("uWaveSuppressionExp");

};