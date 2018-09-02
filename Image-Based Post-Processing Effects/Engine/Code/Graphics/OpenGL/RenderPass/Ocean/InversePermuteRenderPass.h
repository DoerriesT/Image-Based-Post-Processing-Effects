#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct Water;

class InversePermuteRenderPass : public RenderPass
{
public:
	explicit InversePermuteRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const Water &_water, GLuint *_inputTextures, GLuint _displacementTexture, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> inversePermuteShader;
	std::shared_ptr<Mesh> fullscreenTriangle;

	Uniform<GLint> uSimulationResolutionIP = Uniform<GLint>("uN");
	Uniform<GLfloat> uChoppinessIP = Uniform<GLfloat>("uChoppiness");

};