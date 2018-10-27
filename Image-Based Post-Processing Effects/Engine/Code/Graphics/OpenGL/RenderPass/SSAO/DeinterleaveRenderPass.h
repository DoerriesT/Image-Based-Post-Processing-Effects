#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct RenderData;
class Scene;
struct Effects;
struct Level;

class DeinterleaveRenderPass : public RenderPass
{
public:
	explicit DeinterleaveRenderPass(GLuint fbo, unsigned int width, unsigned int height);
	void render(GLuint sourceTexture, GLuint *targetTextures, RenderPass **previousRenderPass = nullptr);
	void resize(unsigned int width, unsigned int height) override;

private:
	std::shared_ptr<ShaderProgram> m_deinterleaveShader;
	std::shared_ptr<Mesh> m_fullscreenTriangle;

	Uniform<glm::vec2> m_uOffset = Uniform<glm::vec2>("uOffset");
};
