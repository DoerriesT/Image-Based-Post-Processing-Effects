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
	std::shared_ptr<ShaderProgram> m_lensFlareGenShader;
	std::shared_ptr<Mesh> m_fullscreenTriangle;

	Uniform<GLint> m_uGhosts = Uniform<GLint>("uGhosts");
	Uniform<GLfloat> m_uGhostDispersal = Uniform<GLfloat>("uGhostDispersal");
	Uniform<GLfloat> m_uHaloRadius = Uniform<GLfloat>("uHaloRadius");
	Uniform<GLfloat> m_uDistortion = Uniform<GLfloat>("uDistortion");
	Uniform<glm::vec4> m_uScale = Uniform<glm::vec4>("uScale");
	Uniform<glm::vec4> m_uBias = Uniform<glm::vec4>("uBias");

};
