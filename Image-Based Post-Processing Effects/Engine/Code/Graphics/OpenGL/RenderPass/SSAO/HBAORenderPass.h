#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct RenderData;
class Scene;
struct Effects;
struct Level;

class HBAORenderPass : public RenderPass
{
public:
	explicit HBAORenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const RenderData &_renderData, const Effects &_effects, GLuint _noiseTexture, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> hbaoShader;
	std::shared_ptr<Mesh> fullscreenTriangle;

	Uniform<glm::vec2> uFocalLengthHBAO = Uniform<glm::vec2>("uFocalLength");
	Uniform<glm::mat4> uInverseProjectionHBAO = Uniform<glm::mat4>("uInverseProjection");
	Uniform<glm::vec2> uAOResHBAO = Uniform<glm::vec2>("uAORes");
	Uniform<glm::vec2> uInvAOResHBAO = Uniform<glm::vec2>("uInvAORes");
	Uniform<glm::vec2> uNoiseScaleHBAO = Uniform<glm::vec2>("uNoiseScale");
	Uniform<GLfloat> uStrengthHBAO = Uniform<GLfloat>("uStrength");
	Uniform<GLfloat> uRadiusHBAO = Uniform<GLfloat>("uRadius");
	Uniform<GLfloat> uRadius2HBAO = Uniform<GLfloat>("uRadius2");
	Uniform<GLfloat> uNegInvR2HBAO = Uniform<GLfloat>("uNegInvR2");
	Uniform<GLfloat> uTanBiasHBAO = Uniform<GLfloat>("uTanBias");
	Uniform<GLfloat> uMaxRadiusPixelsHBAO = Uniform<GLfloat>("uMaxRadiusPixels");
	Uniform<GLfloat> uNumDirectionsHBAO = Uniform<GLfloat>("uNumDirections");
	Uniform<GLfloat> uNumStepsHBAO = Uniform<GLfloat>("uNumSteps");

};
