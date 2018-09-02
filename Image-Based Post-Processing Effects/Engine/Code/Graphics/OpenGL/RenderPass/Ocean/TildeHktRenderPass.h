#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct Water;

class TildeHktRenderPass : public RenderPass
{
public:
	explicit TildeHktRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const Water &_water, GLuint _tildeH0kTexture, GLuint _tildeH0minusKTexture, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> tildeHktShader;
	std::shared_ptr<Mesh> fullscreenTriangle;

	Uniform<GLint> uSimulationResolutionHT = Uniform<GLint>("uN");
	Uniform<GLint> uWorldSizeHT = Uniform<GLint>("uL");
	Uniform<GLfloat> uTimeHT = Uniform<GLfloat>("uTime");

};