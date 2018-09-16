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

class DeferredEnvironmentProbeRenderPass : public RenderPass
{
public:
	explicit DeferredEnvironmentProbeRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const Effects &_effects, const GBuffer &_gbuffer, const GLuint _brdfLUT, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> deferredEnvironmentProbePassShader;
	std::shared_ptr<Mesh> boxMesh;

	Uniform<glm::mat4> uModelViewProjection = Uniform<glm::mat4>("uModelViewProjection");
	Uniform<glm::mat4> uInverseView = Uniform<glm::mat4>("uInverseView");
	Uniform<glm::mat4> uInverseProjection = Uniform<glm::mat4>("uInverseProjection");
	Uniform<glm::vec3> uBoxMin = Uniform<glm::vec3>("uBoxMin");
	Uniform<glm::vec3> uBoxMax = Uniform<glm::vec3>("uBoxMax");
	Uniform<glm::vec3> uProbePosition = Uniform<glm::vec3>("uProbePosition");
};
