#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct Effects;

class SMAAEdgeDetectionRenderPass : public RenderPass
{
public:
	explicit SMAAEdgeDetectionRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const Effects &_effects, GLuint _inputTexture, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> edgeDetectionShader;
	std::shared_ptr<Mesh> fullscreenTriangle;

	Uniform<glm::vec4> uResolutionSMAAE = Uniform<glm::vec4>("uResolution");

};
