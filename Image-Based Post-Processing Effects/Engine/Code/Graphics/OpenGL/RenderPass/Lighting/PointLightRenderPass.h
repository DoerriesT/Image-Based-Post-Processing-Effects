#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct RenderData;
class Scene;
struct Effects;
struct Level;

class PointLightRenderPass : public RenderPass
{
public:
	explicit PointLightRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> pointLightPassShader;
	std::shared_ptr<Mesh> pointLightMesh;

	Uniform<glm::mat4> uModelViewProjectionP = Uniform<glm::mat4>("uModelViewProjection");
	UniformPointLight uPointLightP = UniformPointLight("uPointLight");
	Uniform<glm::mat4> uInverseProjectionP = Uniform<glm::mat4>("uInverseProjection");
	Uniform<glm::mat4> uInverseViewP = Uniform<glm::mat4>("uInverseView");
	Uniform<GLboolean> uShadowsEnabledP = Uniform<GLboolean>("uShadowsEnabled");
	Uniform<glm::vec2> uViewportSizeP = Uniform<glm::vec2>("uViewportSize");
};
