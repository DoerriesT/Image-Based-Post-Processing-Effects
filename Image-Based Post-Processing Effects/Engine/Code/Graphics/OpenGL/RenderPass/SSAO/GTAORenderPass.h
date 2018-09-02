#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"
#include "Graphics\OpenGL\GBuffer.h"

struct RenderData;
class Scene;
struct Effects;
struct Level;

class GTAORenderPass : public RenderPass
{
public:
	explicit GTAORenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const RenderData &_renderData, const Effects &_effects, const GBuffer &_gbuffer, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> gtaoShader;
	std::shared_ptr<Mesh> fullscreenTriangle;

	Uniform<GLfloat> uFocalLengthGTAO = Uniform<GLfloat>("uFocalLength");
	Uniform<glm::mat4> uInverseProjectionGTAO = Uniform<glm::mat4>("uInverseProjection");
	Uniform<glm::vec2> uAOResGTAO = Uniform<glm::vec2>("uAORes");
	Uniform<glm::vec2> uInvAOResGTAO = Uniform<glm::vec2>("uInvAORes");
	Uniform<GLint> uFrameGTAO = Uniform<GLint>("uFrame");
	Uniform<GLfloat> uStrengthGTAO = Uniform<GLfloat>("uStrength");
	Uniform<GLfloat> uRadiusGTAO = Uniform<GLfloat>("uRadius");
	Uniform<GLfloat> uMaxRadiusPixelsGTAO = Uniform<GLfloat>("uMaxRadiusPixels");
	Uniform<GLfloat> uNumStepsGTAO = Uniform<GLfloat>("uNumSteps");

};
