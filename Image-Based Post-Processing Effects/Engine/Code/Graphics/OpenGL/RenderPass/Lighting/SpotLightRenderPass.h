#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct RenderData;
class Scene;
struct Effects;
struct Level;

class SpotLightRenderPass : public RenderPass
{
public:
	explicit SpotLightRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> m_spotLightPassShader;
	std::shared_ptr<Mesh> m_spotLightMesh;

	Uniform<glm::mat4> m_uModelViewProjection = Uniform<glm::mat4>("uModelViewProjection");
	UniformSpotLight m_uSpotLight = UniformSpotLight("uSpotLight");
	Uniform<glm::mat4> m_uInverseProjection = Uniform<glm::mat4>("uInverseProjection");
	Uniform<glm::mat4> m_uInverseView = Uniform<glm::mat4>("uInverseView");
	Uniform<GLboolean> m_uShadowsEnabled = Uniform<GLboolean>("uShadowsEnabled");
	Uniform<glm::vec2> m_uViewportSize = Uniform<glm::vec2>("uViewportSize");
};
