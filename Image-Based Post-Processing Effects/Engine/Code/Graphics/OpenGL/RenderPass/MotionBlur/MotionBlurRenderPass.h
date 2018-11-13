#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

class MotionBlurRenderPass : public RenderPass
{
public:
	explicit MotionBlurRenderPass(GLuint fbo, unsigned int width, unsigned int height);
	void render(int motionBlurType, RenderPass **previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> m_motionBlurShader;
	std::shared_ptr<Mesh> m_fullscreenTriangle;

};
