#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

class LensFlareBlurRenderPass : public RenderPass
{
public:
	explicit LensFlareBlurRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(GLuint _inputTexture, GLuint _pingPongTexture, RenderPass **_previousRenderPass = nullptr);
	void resize(unsigned int _width, unsigned int _height) override;

private:
	std::shared_ptr<ShaderProgram> m_lensFlareBlurShader;
	std::shared_ptr<Mesh> m_fullscreenTriangle;

	Uniform<GLboolean> m_uDirection = Uniform<GLboolean>("uDirection");

};
