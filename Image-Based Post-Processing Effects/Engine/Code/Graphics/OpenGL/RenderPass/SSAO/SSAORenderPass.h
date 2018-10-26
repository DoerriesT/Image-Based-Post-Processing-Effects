#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct RenderData;
class Scene;
struct Effects;
struct Level;

class SSAORenderPass : public RenderPass
{
public:
	explicit SSAORenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const RenderData &_renderData, const Effects &_effects, GLuint _noiseTexture, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> m_ssaoShader;
	std::shared_ptr<Mesh> m_fullscreenTriangle;

	Uniform<glm::mat4> m_uView = Uniform<glm::mat4>("uView");
	Uniform<glm::mat4> m_uProjection = Uniform<glm::mat4>("uProjection");
	Uniform<glm::mat4> m_uInverseProjection = Uniform<glm::mat4>("uInverseProjection");
	std::vector<GLint> m_uSamples;
	Uniform<GLint> m_uKernelSize = Uniform<GLint>("uKernelSize");
	Uniform<GLfloat> m_uRadius = Uniform<GLfloat>("uRadius");
	Uniform<GLfloat> m_uBias = Uniform<GLfloat>("uBias");
	Uniform<GLfloat> m_uStrength = Uniform<GLfloat>("uStrength");

};
