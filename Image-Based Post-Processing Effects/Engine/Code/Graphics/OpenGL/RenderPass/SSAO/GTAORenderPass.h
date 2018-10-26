#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct RenderData;
class Scene;
struct Effects;
struct Level;

class GTAORenderPass : public RenderPass
{
public:
	explicit GTAORenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const RenderData &_renderData, const Effects &_effects, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> m_gtaoShader;
	std::shared_ptr<Mesh> m_fullscreenTriangle;

	Uniform<GLfloat> m_uFocalLength = Uniform<GLfloat>("uFocalLength");
	Uniform<glm::mat4> m_uInverseProjection = Uniform<glm::mat4>("uInverseProjection");
	Uniform<glm::vec2> m_uAORes = Uniform<glm::vec2>("uAORes");
	Uniform<glm::vec2> m_uInvAORes = Uniform<glm::vec2>("uInvAORes");
	Uniform<GLint> m_uFrame = Uniform<GLint>("uFrame");
	Uniform<GLfloat> m_uStrength = Uniform<GLfloat>("uStrength");
	Uniform<GLfloat> m_uRadius = Uniform<GLfloat>("uRadius");
	Uniform<GLfloat> m_uMaxRadiusPixels = Uniform<GLfloat>("uMaxRadiusPixels");
	Uniform<GLfloat> m_uNumSteps = Uniform<GLfloat>("uNumSteps");

};
