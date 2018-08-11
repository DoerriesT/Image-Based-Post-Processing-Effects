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

class OutlineRenderPass : public RenderPass
{
public:
	explicit OutlineRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const RenderData &_renderData, const Scene &_scene, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> outlineShader;

	Uniform<glm::mat4> uModelViewProjectionMatrixO = Uniform<glm::mat4>("uModelViewProjectionMatrix");
	Uniform<glm::vec4> uOutlineColorO = Uniform<glm::vec4>("uOutlineColor");
};
