#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct OceanParams;

class ButterflyPrecomputeRenderPass : public RenderPass
{
public:
	explicit ButterflyPrecomputeRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const OceanParams &_water, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> m_butterflyPrecomputeShader;
	std::shared_ptr<Mesh> m_fullscreenTriangle;

	std::vector<GLint> m_uJ;
	Uniform<GLint> m_uSimulationResolution = Uniform<GLint>("uN");

};