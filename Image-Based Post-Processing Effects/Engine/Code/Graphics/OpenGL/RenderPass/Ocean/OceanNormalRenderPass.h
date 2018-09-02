#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct Water;

class OceanNormalRenderPass : public RenderPass
{
public:
	explicit OceanNormalRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const Water &_water, GLuint _displacementTexture, GLuint _normalTexture, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> oceanNormalShader;
	std::shared_ptr<Mesh> fullscreenTriangle;

	Uniform<GLfloat> uNormalStrengthN = Uniform<GLfloat>("uNormalStrength");

};