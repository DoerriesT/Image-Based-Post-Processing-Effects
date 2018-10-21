#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"

struct RenderData;
class Scene;
struct Level;

class ShadowRenderPass : public RenderPass
{
public:
	explicit ShadowRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const Scene &_scene, bool _cascadeSkipOptimization, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> shadowShader;

	Uniform<glm::mat4> uModelViewProjectionMatrix = Uniform<glm::mat4>("uModelViewProjectionMatrix");

	void renderShadows(const glm::mat4 &_viewProjectionMatrix, const Scene &_scene);
	glm::mat4 calculateLightViewProjection(const RenderData &_renderData, const glm::vec3 &_lightDir, float _nearPlane, float _farPlane);
};
