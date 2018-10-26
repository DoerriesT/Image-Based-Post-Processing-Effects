#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct RenderData;
class Scene;
struct Effects;
struct Level;

class SkyboxRenderPass : public RenderPass
{
public:
	explicit SkyboxRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> m_skyboxShader;
	std::shared_ptr<Mesh> m_fullscreenTriangle;

	Uniform<glm::mat4> m_uInverseModelViewProjection = Uniform<glm::mat4>("uInverseModelViewProjection");
	Uniform<glm::mat4> m_uCurrentToPrevTransform = Uniform<glm::mat4>("uCurrentToPrevTransform");
	Uniform<glm::vec4> m_uColor = Uniform<glm::vec4>("uColor");
	Uniform<GLboolean> m_uHasAlbedoMap = Uniform<GLboolean>("uHasAlbedoMap");
};
