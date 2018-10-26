#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct OceanParams;

class InversePermuteRenderPass : public RenderPass
{
public:
	explicit InversePermuteRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const OceanParams &_water, GLuint *_inputTextures, GLuint _displacementTexture, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> m_inversePermuteShader;
	std::shared_ptr<Mesh> m_fullscreenTriangle;

	Uniform<GLint> m_uSimulationResolution = Uniform<GLint>("uN");
	Uniform<GLfloat> m_uChoppiness = Uniform<GLfloat>("uChoppiness");

};