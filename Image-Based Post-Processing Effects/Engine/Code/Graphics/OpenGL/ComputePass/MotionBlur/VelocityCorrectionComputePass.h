#pragma once
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"

struct Effects;
struct RenderData;

class VelocityCorrectionComputePass
{
public:
	explicit VelocityCorrectionComputePass(unsigned int _width, unsigned int _height);
	void execute(const RenderData & _renderData, GLuint _velocityTexture, GLuint _depthTexture);
	void resize(unsigned int _width, unsigned int _height);

private:
	std::shared_ptr<ShaderProgram> m_velocityCorrectionShader;

	Uniform<glm::mat4> m_uReprojection = Uniform<glm::mat4>("uReprojection");
	Uniform<GLfloat> m_uScale = Uniform<GLfloat>("uScale");

	unsigned int m_width;
	unsigned int m_height;
};