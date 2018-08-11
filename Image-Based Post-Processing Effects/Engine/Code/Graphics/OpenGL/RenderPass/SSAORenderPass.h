#pragma once
#include "RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"
#include "Graphics\OpenGL\GBuffer.h"

struct RenderData;
class Scene;
struct Effects;
struct Level;

class SSAORenderPass : public RenderPass
{
public:
	explicit SSAORenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const RenderData &_renderData, const Effects &_effects, const GBuffer &_gbuffer, GLuint _noiseTexture, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> ssaoShader;
	std::shared_ptr<Mesh> fullscreenTriangle;

	Uniform<glm::mat4> uViewAO = Uniform<glm::mat4>("uView");
	Uniform<glm::mat4> uProjectionAO = Uniform<glm::mat4>("uProjection");
	Uniform<glm::mat4> uInverseProjectionAO = Uniform<glm::mat4>("uInverseProjection");
	std::vector<GLint> uSamplesAO;
	Uniform<GLint> uKernelSizeAO = Uniform<GLint>("uKernelSize");
	Uniform<GLfloat> uRadiusAO = Uniform<GLfloat>("uRadius");
	Uniform<GLfloat> uBiasAO = Uniform<GLfloat>("uBias");
	Uniform<GLfloat> uStrengthAO = Uniform<GLfloat>("uStrength");

};
