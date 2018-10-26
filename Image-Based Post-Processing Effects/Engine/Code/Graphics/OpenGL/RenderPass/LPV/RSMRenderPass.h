#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"

struct RenderData;
class Scene;
struct Level;

class RSMRenderPass : public RenderPass
{
public:
	explicit RSMRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const glm::mat4 &_viewProjection, std::shared_ptr<DirectionalLight> _light, const Scene &_scene, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> m_rsmPassShader;

	Uniform<glm::mat3> m_uModelMatrix = Uniform<glm::mat3>("uModelMatrix");
	Uniform<glm::mat4> m_uModelViewProjectionMatrix = Uniform<glm::mat4>("uModelViewProjectionMatrix");
	Uniform<glm::vec4> m_uAtlasData = Uniform<glm::vec4>("uAtlasData");
	Uniform<GLboolean> m_uHasTexture = Uniform<GLboolean>("uHasTexture");
	Uniform<glm::vec3> m_uAlbedo = Uniform<glm::vec3>("uAlbedo");
	Uniform<glm::vec3> m_uLightColor = Uniform<glm::vec3>("uLightColor");
	Uniform<glm::vec3> m_uLightDir = Uniform<glm::vec3>("uLightDir");
};
