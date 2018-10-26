#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct RenderData;
struct Level;

class LightProbeRenderPass : public RenderPass
{
public:
	explicit LightProbeRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> m_lightProbeShader;
	std::shared_ptr<Mesh> m_sphereMesh;

	Uniform<glm::mat4> m_uModelViewProjectionMatrix = Uniform<glm::mat4>("uModelViewProjectionMatrix");
	Uniform<GLboolean> m_uSH = Uniform<GLboolean>("uSH");
	Uniform<GLint> m_uIndex = Uniform<GLint>("uIndex");
};
