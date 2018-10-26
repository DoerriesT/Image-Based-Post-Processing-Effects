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
	std::shared_ptr<ShaderProgram> lightProbeShader;
	std::shared_ptr<Mesh> sphereMesh;

	Uniform<glm::mat4> uModelViewProjectionMatrix = Uniform<glm::mat4>("uModelViewProjectionMatrix");
	Uniform<GLboolean> uSH = Uniform<GLboolean>("uSH");
	Uniform<GLint> uIndex = Uniform<GLint>("uIndex");
};
