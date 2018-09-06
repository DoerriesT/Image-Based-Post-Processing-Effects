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
	void render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const Scene &_scene, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> rsmPassShader;

	Uniform<glm::mat3> uModelMatrix = Uniform<glm::mat3>("uModelMatrix");
	Uniform<glm::mat4> uModelViewProjectionMatrix = Uniform<glm::mat4>("uModelViewProjectionMatrix");
	Uniform<glm::vec4> uAtlasData = Uniform<glm::vec4>("uAtlasData");
	Uniform<GLboolean> uHasTexture = Uniform<GLboolean>("uHasTexture");
	Uniform<glm::vec3> uAlbedo = Uniform<glm::vec3>("uAlbedo");
	Uniform<glm::vec3> uLightColor = Uniform<glm::vec3>("uLightColor");
	Uniform<glm::vec3> uLightDir = Uniform<glm::vec3>("uLightDir");
};
