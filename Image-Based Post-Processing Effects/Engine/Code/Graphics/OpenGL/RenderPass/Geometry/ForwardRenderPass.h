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
	std::shared_ptr<ShaderProgram> transparencyShader;

	Uniform<glm::mat4> uViewMatrixT = Uniform<glm::mat4>("uViewMatrix");
	Uniform<glm::mat4> uPrevTransformT = Uniform<glm::mat4>("uPrevTransform");
	Uniform<glm::mat4> uCurrTransformT = Uniform<glm::mat4>("uCurrTransform");
	Uniform<glm::mat4> uModelViewProjectionMatrixT = Uniform<glm::mat4>("uModelViewProjectionMatrix");
	Uniform<glm::mat4> uModelMatrixT = Uniform<glm::mat4>("uModelMatrix");
	Uniform<glm::vec4> uAtlasDataT = Uniform<glm::vec4>("uAtlasData");
	UniformMaterial uMaterialT = UniformMaterial("uMaterial");
	UniformDirectionalLight uDirectionalLightT = UniformDirectionalLight("uDirectionalLight");
	Uniform<GLboolean> uRenderDirectionalLightT = Uniform<GLboolean>("uRenderDirectionalLight");
	Uniform<glm::vec3> uCamPosT = Uniform<glm::vec3>("uCamPos");
	Uniform<GLboolean> uShadowsEnabledT = Uniform<GLboolean>("uShadowsEnabled");
};
