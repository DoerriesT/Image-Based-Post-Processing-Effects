#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct RenderData;
class Scene;
struct Effects;
struct Level;

class BoundingBoxRenderPass : public RenderPass
{
public:
	explicit BoundingBoxRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const Scene &_scene, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> m_boundingBoxShader;
	std::shared_ptr<Mesh> boxMesh;

	Uniform<glm::mat4> m_uModelViewProjectionMatrix = Uniform<glm::mat4>("uModelViewProjectionMatrix");
	Uniform<glm::vec3> m_uColor = Uniform<glm::vec3>("uColor");

};
