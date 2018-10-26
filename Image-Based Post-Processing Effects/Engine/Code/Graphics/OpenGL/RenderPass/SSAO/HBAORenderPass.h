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
	std::shared_ptr<ShaderProgram> m_hbaoShader;
	std::shared_ptr<Mesh> m_fullscreenTriangle;

	Uniform<glm::vec2> m_uFocalLength = Uniform<glm::vec2>("uFocalLength");
	Uniform<glm::mat4> m_uInverseProjection = Uniform<glm::mat4>("uInverseProjection");
	Uniform<glm::vec2> m_uAORes = Uniform<glm::vec2>("uAORes");
	Uniform<glm::vec2> m_uInvAORes = Uniform<glm::vec2>("uInvAORes");
	Uniform<glm::vec2> m_uNoiseScale = Uniform<glm::vec2>("uNoiseScale");
	Uniform<GLfloat> m_uStrength = Uniform<GLfloat>("uStrength");
	Uniform<GLfloat> m_uRadius = Uniform<GLfloat>("uRadius");
	Uniform<GLfloat> m_uRadius2 = Uniform<GLfloat>("uRadius2");
	Uniform<GLfloat> m_uNegInvR2 = Uniform<GLfloat>("uNegInvR2");
	Uniform<GLfloat> m_uTanBias = Uniform<GLfloat>("uTanBias");
	Uniform<GLfloat> m_uMaxRadiusPixels = Uniform<GLfloat>("uMaxRadiusPixels");
	Uniform<GLfloat> m_uNumDirections = Uniform<GLfloat>("uNumDirections");
	Uniform<GLfloat> m_uNumSteps = Uniform<GLfloat>("uNumSteps");

};
