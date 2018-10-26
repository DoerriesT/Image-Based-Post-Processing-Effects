#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct RenderData;
class Scene;
struct Effects;
struct Level;

class DeferredEnvironmentProbeRenderPass : public RenderPass
{
public:
	explicit DeferredEnvironmentProbeRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const Effects &_effects, GLuint _ssaoTexture, GLuint _brdfLUT, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> m_deferredEnvironmentProbePassShader;
	std::shared_ptr<Mesh> m_boxMesh;

	Uniform<glm::mat4> m_uModelViewProjection = Uniform<glm::mat4>("uModelViewProjection");
	Uniform<glm::mat4> m_uInverseView = Uniform<glm::mat4>("uInverseView");
	Uniform<glm::mat4> m_uInverseProjection = Uniform<glm::mat4>("uInverseProjection");
	Uniform<glm::vec3> m_uBoxMin = Uniform<glm::vec3>("uBoxMin");
	Uniform<glm::vec3> m_uBoxMax = Uniform<glm::vec3>("uBoxMax");
	Uniform<glm::vec3> m_uProbePosition = Uniform<glm::vec3>("uProbePosition");
};
