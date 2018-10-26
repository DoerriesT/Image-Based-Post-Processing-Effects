#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

class BloomDownsampleRenderPass : public RenderPass
{
public:
	explicit BloomDownsampleRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(GLuint *_textureChain, size_t _textureCount, GLuint *_fbos, size_t _fboCount, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> m_downsampleShader;
	std::shared_ptr<Mesh> m_fullscreenTriangle;

};
