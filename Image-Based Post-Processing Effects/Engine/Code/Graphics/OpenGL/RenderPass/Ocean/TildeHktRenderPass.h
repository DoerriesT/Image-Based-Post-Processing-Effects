#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct OceanParams;

class TildeHktRenderPass : public RenderPass
{
public:
	explicit TildeHktRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const OceanParams &_water, GLuint _tildeH0kTexture, GLuint _tildeH0minusKTexture, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> m_tildeHktShader;
	std::shared_ptr<Mesh> m_fullscreenTriangle;

	Uniform<GLint> m_uSimulationResolution = Uniform<GLint>("uN");
	Uniform<GLint> m_uWorldSize = Uniform<GLint>("uL");
	Uniform<GLfloat> m_uTime = Uniform<GLfloat>("uTime");

};