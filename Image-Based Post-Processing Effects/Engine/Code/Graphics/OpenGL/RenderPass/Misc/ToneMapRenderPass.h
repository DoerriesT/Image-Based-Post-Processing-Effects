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
	std::shared_ptr<ShaderProgram> toneMapShader;
	std::shared_ptr<Mesh> fullscreenTriangle;

	Uniform<GLfloat> uStarburstOffsetH = Uniform<GLfloat>("uStarburstOffset");
	Uniform<GLfloat> uBloomStrengthH = Uniform<GLfloat>("uBloomStrength");
	Uniform<GLfloat> uLensDirtStrengthH = Uniform<GLfloat>("uLensDirtStrength");
	Uniform<GLfloat> uExposureH = Uniform<GLfloat>("uExposure");
	Uniform<glm::vec3> uAnamorphicFlareColorH = Uniform<glm::vec3>("uAnamorphicFlareColor");

};
