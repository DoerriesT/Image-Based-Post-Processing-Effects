#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

class BloomUpsampleRenderPass : public RenderPass
{
public:
	explicit BloomUpsampleRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(GLuint *_sourceTextureChain, size_t _sourceTextureCount, GLuint *_targetTextureChain, size_t targetTextureCount, GLuint *_fbos, size_t _fboCount, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> m_upsampleShader;
	std::shared_ptr<Mesh> m_fullscreenTriangle;

	Uniform<GLboolean> m_uAddPreviousBU = Uniform<GLboolean>("uAddPrevious");
	Uniform<glm::vec2> m_uRadiusBU = Uniform<glm::vec2>("uRadius");
};
