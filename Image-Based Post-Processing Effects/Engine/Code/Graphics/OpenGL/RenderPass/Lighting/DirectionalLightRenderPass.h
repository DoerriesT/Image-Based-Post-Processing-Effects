#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct RenderData;
class Scene;
struct Effects;
struct Level;

class DirectionalLightRenderPass : public RenderPass
{
public:
	explicit DirectionalLightRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> directionalLightShader;
	std::shared_ptr<Mesh> fullscreenTriangle;

	UniformDirectionalLight uDirectionalLightD = UniformDirectionalLight("uDirectionalLight");
	Uniform<glm::mat4> uInverseViewD = Uniform<glm::mat4>("uInverseView");
	Uniform<glm::mat4> uInverseProjectionD = Uniform<glm::mat4>("uInverseProjection");
	Uniform<GLboolean> uShadowsEnabledD = Uniform<GLboolean>("uShadowsEnabled");
};
