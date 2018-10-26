#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct OceanParams;

class ButterflyComputeRenderPass : public RenderPass
{
public:
	explicit ButterflyComputeRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const OceanParams &_water, GLuint _twiddleIndicesTexture, GLuint *_readTextures, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> m_butterflyComputeShader;
	std::shared_ptr<Mesh> m_fullscreenTriangle;

	Uniform<GLint> m_uSimulationResolution = Uniform<GLint>("uN");
	Uniform<GLint> m_uStage = Uniform<GLint>("uStage");
	Uniform<GLint> m_uStages = Uniform<GLint>("uStages");
	Uniform<GLint> m_uDirection = Uniform<GLint>("uDirection");

};