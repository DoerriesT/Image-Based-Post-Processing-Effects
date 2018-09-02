#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct Water;

class ButterflyPrecomputeRenderPass : public RenderPass
{
public:
	explicit ButterflyPrecomputeRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const Water &_water, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> butterflyPrecomputeShader;
	std::shared_ptr<Mesh> fullscreenTriangle;

	std::vector<GLint> uJBP;
	Uniform<GLint> uSimulationResolutionBP = Uniform<GLint>("uN");

};