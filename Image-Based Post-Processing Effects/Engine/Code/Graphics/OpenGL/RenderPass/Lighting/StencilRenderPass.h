#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"
#include "Graphics\OpenGL\GBuffer.h"

struct RenderData;
class Scene;
struct Effects;
struct Level;

class StencilRenderPass : public RenderPass
{
public:
	explicit StencilRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const GBuffer &_gbuffer, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> stencilPassShader;
	std::shared_ptr<Mesh> pointLightMesh;
	std::shared_ptr<Mesh> spotLightMesh;
	std::shared_ptr<Mesh> boxMesh;

	Uniform<glm::mat4> uModelViewProjection = Uniform<glm::mat4>("uModelViewProjection");
};
