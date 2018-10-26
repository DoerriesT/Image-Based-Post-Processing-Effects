#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"

struct RenderData;
class Scene;

class GBufferRenderPass : public RenderPass
{
public:
	explicit GBufferRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const RenderData &_renderData, const Scene &_scene, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> m_gBufferPassShader;

	Uniform<glm::vec3> m_uCamPos = Uniform<glm::vec3>("uCamPos");
	Uniform<glm::mat3> m_uViewMatrix = Uniform<glm::mat3>("uViewMatrix");
	Uniform<glm::mat4> m_uModelMatrix = Uniform<glm::mat4>("uModelMatrix");
	Uniform<glm::mat4> m_uModelViewProjectionMatrix = Uniform<glm::mat4>("uModelViewProjectionMatrix");
	Uniform<glm::mat4> m_uPrevTransform = Uniform<glm::mat4>("uPrevTransform");
	Uniform<glm::mat4> m_uCurrTransform = Uniform<glm::mat4>("uCurrTransform");
	Uniform<glm::vec4> m_uAtlasData = Uniform<glm::vec4>("uAtlasData");
	Uniform<glm::vec2> m_uVel = Uniform<glm::vec2>("uVel");
	Uniform<GLfloat> m_uExposureTime = Uniform<GLfloat>("uExposureTime");
	Uniform<GLfloat> m_uMaxVelocityMag = Uniform<GLfloat>("uMaxVelocityMag");
	UniformMaterial m_uMaterial = UniformMaterial("uMaterial");
};
