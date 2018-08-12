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

class SSAOBilateralBlurRenderPass : public RenderPass
{
public:
	explicit SSAOBilateralBlurRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const RenderData &_renderData, const Effects &_effects, const GBuffer &_gbuffer, GLuint *_ssaoTextures, float _sharpness, float _radius, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> ssaoBilateralBlurShader;
	std::shared_ptr<Mesh> fullscreenTriangle;

	Uniform<GLfloat> uSharpnessAOBB = Uniform<GLfloat>("uSharpness");
	Uniform<GLfloat> uKernelRadiusAOBB = Uniform<GLfloat>("uKernelRadius");
	Uniform<glm::vec2> uInvResolutionDirectionAOBB = Uniform<glm::vec2>("uInvResolutionDirection");

};
