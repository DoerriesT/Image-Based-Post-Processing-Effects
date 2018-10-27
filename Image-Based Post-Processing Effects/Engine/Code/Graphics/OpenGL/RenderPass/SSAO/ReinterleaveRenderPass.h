#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct RenderData;
class Scene;
struct Effects;
struct Level;

class ReinterleaveRenderPass : public RenderPass
{
public:
	explicit ReinterleaveRenderPass(GLuint fbo, unsigned int width, unsigned int height);
	void render(GLuint sourceArrayTexture, GLuint targetTexture, RenderPass **previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> m_reinterleaveShader;
	std::shared_ptr<Mesh> m_fullscreenTriangle;
};
