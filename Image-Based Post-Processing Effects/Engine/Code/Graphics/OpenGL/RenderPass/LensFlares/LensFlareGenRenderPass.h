#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct Effects;

class LensFlareGenRenderPass : public RenderPass
{
public:
	explicit LensFlareGenRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const Effects &_effects, GLuint _inputTexture, RenderPass **_previousRenderPass = nullptr);
	void resize(unsigned int _width, unsigned int _height) override;

private:
	std::shared_ptr<ShaderProgram> lensFlareGenShader;
	std::shared_ptr<Mesh> fullscreenTriangle;

	Uniform<GLint> uGhostsLFG = Uniform<GLint>("uGhosts");
	Uniform<GLfloat> uGhostDispersalLFG = Uniform<GLfloat>("uGhostDispersal");
	Uniform<GLfloat> uHaloRadiusLFG = Uniform<GLfloat>("uHaloRadius");
	Uniform<GLfloat> uDistortionLFG = Uniform<GLfloat>("uDistortion");
	Uniform<glm::vec4> uScaleLFG = Uniform<glm::vec4>("uScale");
	Uniform<glm::vec4> uBiasLFG = Uniform<glm::vec4>("uBias");

};
