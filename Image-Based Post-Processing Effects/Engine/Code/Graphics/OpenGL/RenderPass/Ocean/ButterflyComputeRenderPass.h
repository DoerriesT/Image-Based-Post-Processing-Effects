#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct Water;

class ButterflyComputeRenderPass : public RenderPass
{
public:
	explicit ButterflyComputeRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const Water &_water, GLuint _twiddleIndicesTexture, GLuint *_readTextures, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> butterflyComputeShader;
	std::shared_ptr<Mesh> fullscreenTriangle;

	Uniform<GLint> uSimulationResolutionBC = Uniform<GLint>("uN");
	Uniform<GLint> uStageBC = Uniform<GLint>("uStage");
	Uniform<GLint> uStagesBC = Uniform<GLint>("uStages");
	Uniform<GLint> uDirectionBC = Uniform<GLint>("uDirection");

};