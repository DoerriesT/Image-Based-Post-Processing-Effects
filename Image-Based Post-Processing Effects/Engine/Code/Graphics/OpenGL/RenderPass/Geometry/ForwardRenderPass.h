#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct RenderData;
class Scene;
struct Effects;
struct Level;

class ForwardRenderPass : public RenderPass
{
public:
	explicit ForwardRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const Scene &_scene, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> m_forwardShader;

	Uniform<glm::mat4> m_uViewMatrix = Uniform<glm::mat4>("uViewMatrix");
	Uniform<glm::mat4> m_uPrevTransform = Uniform<glm::mat4>("uPrevTransform");
	Uniform<glm::mat4> m_uCurrTransform = Uniform<glm::mat4>("uCurrTransform");
	Uniform<glm::mat4> m_uModelViewProjectionMatrix = Uniform<glm::mat4>("uModelViewProjectionMatrix");
	Uniform<glm::mat4> m_uModelMatrix = Uniform<glm::mat4>("uModelMatrix");
	Uniform<glm::vec4> m_uAtlasData = Uniform<glm::vec4>("uAtlasData");
	UniformMaterial m_uMaterial = UniformMaterial("uMaterial");
	UniformDirectionalLight m_uDirectionalLight = UniformDirectionalLight("uDirectionalLight");
	Uniform<GLboolean> m_uRenderDirectionalLight = Uniform<GLboolean>("uRenderDirectionalLight");
	Uniform<glm::vec3> m_uCamPos = Uniform<glm::vec3>("uCamPos");
	Uniform<GLboolean> m_uShadowsEnabled = Uniform<GLboolean>("uShadowsEnabled");
};
