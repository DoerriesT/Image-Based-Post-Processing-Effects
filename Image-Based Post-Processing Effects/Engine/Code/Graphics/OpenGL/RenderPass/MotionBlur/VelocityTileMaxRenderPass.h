#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

class VelocityTileMaxRenderPass : public RenderPass
{
public:
	explicit VelocityTileMaxRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(GLuint _inputVelocityTexture, GLuint _intermediaryTexture, GLuint _velocityTileMaxTexture, unsigned int _tileSize, RenderPass **_previousRenderPass = nullptr);
	void resize(unsigned int _width, unsigned int _height) override;

private:
	std::shared_ptr<ShaderProgram> m_tileMaxShader;
	std::shared_ptr<Mesh> m_fullscreenTriangle;

	Uniform<GLboolean> m_uDirection = Uniform<GLboolean>("uDirection");
	Uniform<GLint> m_uTileSize = Uniform<GLint>("uTileSize");

	unsigned int m_width;
	unsigned int m_height;
};
