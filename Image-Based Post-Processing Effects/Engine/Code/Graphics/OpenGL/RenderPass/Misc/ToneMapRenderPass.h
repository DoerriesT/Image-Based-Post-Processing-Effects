#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct Effects;

class ToneMapRenderPass : public RenderPass
{
public:
	explicit ToneMapRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const Effects &_effects, float _starburstOffset, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> m_toneMapShader;
	std::shared_ptr<Mesh> m_fullscreenTriangle;

	Uniform<GLfloat> m_uStarburstOffset = Uniform<GLfloat>("uStarburstOffset");
	Uniform<GLfloat> m_uBloomStrength = Uniform<GLfloat>("uBloomStrength");
	Uniform<GLfloat> m_uLensDirtStrength = Uniform<GLfloat>("uLensDirtStrength");
	Uniform<GLfloat> m_uExposure = Uniform<GLfloat>("uExposure");
	Uniform<glm::vec3> m_uAnamorphicFlareColor = Uniform<glm::vec3>("uAnamorphicFlareColor");

};
