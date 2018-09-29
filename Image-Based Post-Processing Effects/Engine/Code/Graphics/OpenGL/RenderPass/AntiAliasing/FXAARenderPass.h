#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct Effects;

class FXAARenderPass : public RenderPass
{
public:
	explicit FXAARenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const Effects &_effects, GLuint _inputTexture, GLenum _drawBuffer, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> fxaaShader;
	std::shared_ptr<Mesh> fullscreenTriangle;

	Uniform<glm::vec2> uInverseResolutionF = Uniform<glm::vec2>("uInverseResolution");
	Uniform<GLfloat> uSubPixelAAF = Uniform<GLfloat>("uSubPixelAA");
	Uniform<GLfloat> uEdgeThresholdF = Uniform<GLfloat>("uEdgeThreshold");
	Uniform<GLfloat> uEdgeThresholdMinF = Uniform<GLfloat>("uEdgeThresholdMin");

};
