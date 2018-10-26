#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct RenderData;
class Scene;
struct Effects;
struct Level;

class StencilRenderPass : public RenderPass
{
public:
	explicit StencilRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> m_stencilPassShader;
	std::shared_ptr<Mesh> m_pointLightMesh;
	std::shared_ptr<Mesh> m_spotLightMesh;
	std::shared_ptr<Mesh> m_boxMesh;

	Uniform<glm::mat4> m_uModelViewProjection = Uniform<glm::mat4>("uModelViewProjection");
};
