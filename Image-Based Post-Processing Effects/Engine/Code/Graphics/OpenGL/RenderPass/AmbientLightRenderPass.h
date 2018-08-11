#pragma once
#include "RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"
#include "Graphics\OpenGL\GBuffer.h"

struct RenderData;
class Scene;
struct Effects;
struct Level;

class AmbientLightRenderPass : public RenderPass
{
public:
	explicit AmbientLightRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const Effects &_effects, const GBuffer &_gbuffer, GLuint _brdfLUT, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> environmentLightPassShader;
	std::shared_ptr<Mesh> fullscreenTriangle;

	Uniform<glm::mat4> uProjectionE = Uniform<glm::mat4>("uProjection");
	Uniform<glm::mat4> uInverseProjectionE = Uniform<glm::mat4>("uInverseProjection");
	Uniform<glm::mat4> uInverseViewE = Uniform<glm::mat4>("uInverseView");
	Uniform<glm::mat4> uReProjectionE = Uniform<glm::mat4>("uReProjection");
	UniformDirectionalLight uDirectionalLightE = UniformDirectionalLight("uDirectionalLight");
	Uniform<GLboolean> uShadowsEnabledE = Uniform<GLboolean>("uShadowsEnabled");
	Uniform<GLboolean> uRenderDirectionalLightE = Uniform<GLboolean>("uRenderDirectionalLight");
	Uniform<GLboolean> uSsaoE = Uniform<GLboolean>("uSsao");
	Uniform<GLboolean> uUseSsrE = Uniform<GLboolean>("uUseSsr");
};
