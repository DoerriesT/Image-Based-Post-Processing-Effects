#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

class VelocityNeighborTileMaxRenderPass : public RenderPass
{
public:
	explicit VelocityNeighborTileMaxRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(GLuint _velocityTileMaxTexture, GLuint _velocityNeighborTileMaxTexture, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> m_neighborTileMaxShader;
	std::shared_ptr<Mesh> m_fullscreenTriangle;
};
