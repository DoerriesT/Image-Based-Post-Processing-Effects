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
	std::shared_ptr<ShaderProgram> m_directionalLightShader;
	std::shared_ptr<Mesh> m_fullscreenTriangle;

	UniformDirectionalLight m_uDirectionalLight = UniformDirectionalLight("uDirectionalLight");
	Uniform<glm::mat4> m_uInverseView = Uniform<glm::mat4>("uInverseView");
	Uniform<glm::mat4> m_uInverseProjection = Uniform<glm::mat4>("uInverseProjection");
	Uniform<GLboolean> m_uShadowsEnabled = Uniform<GLboolean>("uShadowsEnabled");
};
