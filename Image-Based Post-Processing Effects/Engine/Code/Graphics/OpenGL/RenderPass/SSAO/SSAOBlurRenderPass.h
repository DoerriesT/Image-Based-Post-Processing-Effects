#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct RenderData;
class Scene;
struct Effects;
struct Level;

class SSAOBlurRenderPass : public RenderPass
{
public:
	explicit SSAOBlurRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const RenderData &_renderData, const Effects &_effects, GLuint _inputTexture, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> m_ssaoBlurShader;
	std::shared_ptr<Mesh> m_fullscreenTriangle;

	Uniform<GLint> m_uBlurSize = Uniform<GLint>("uBlurSize"); // size of noise texture

};
