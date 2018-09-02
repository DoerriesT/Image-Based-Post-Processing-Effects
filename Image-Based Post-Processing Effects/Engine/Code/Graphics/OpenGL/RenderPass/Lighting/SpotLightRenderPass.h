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

class SpotLightRenderPass : public RenderPass
{
public:
	explicit SpotLightRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const GBuffer &_gbuffer, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> spotLightPassShader;
	std::shared_ptr<Mesh> spotLightMesh;

	Uniform<glm::mat4> uModelViewProjectionS = Uniform<glm::mat4>("uModelViewProjection");
	UniformSpotLight uSpotLightS = UniformSpotLight("uSpotLight");
	Uniform<glm::mat4> uInverseProjectionS = Uniform<glm::mat4>("uInverseProjection");
	Uniform<glm::mat4> uInverseViewS = Uniform<glm::mat4>("uInverseView");
	Uniform<GLboolean> uShadowsEnabledS = Uniform<GLboolean>("uShadowsEnabled");
	Uniform<glm::vec2> uViewportSizeS = Uniform<glm::vec2>("uViewportSize");
};
