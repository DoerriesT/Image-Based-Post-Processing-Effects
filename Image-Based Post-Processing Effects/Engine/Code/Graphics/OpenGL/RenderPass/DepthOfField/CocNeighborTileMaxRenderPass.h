#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

class CocNeighborTileMaxRenderPass : public RenderPass
{
public:
	explicit CocNeighborTileMaxRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(GLuint _cocTileMaxTexture, GLuint _cocNeighborTileMaxTexture, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> m_neighborTileMaxShader;
	std::shared_ptr<Mesh> m_fullscreenTriangle;
};
