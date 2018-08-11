#pragma once
#include "RenderPass.h"
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
	std::shared_ptr<ShaderProgram> gBufferPassShader;

	Uniform<glm::vec3> uCamPosG = Uniform<glm::vec3>("uCamPos");
	Uniform<glm::mat3> uViewMatrixG = Uniform<glm::mat3>("uViewMatrix");
	Uniform<glm::mat4> uModelMatrixG = Uniform<glm::mat4>("uModelMatrix");
	Uniform<glm::mat4> uModelViewProjectionMatrixG = Uniform<glm::mat4>("uModelViewProjectionMatrix");
	Uniform<glm::mat4> uPrevTransformG = Uniform<glm::mat4>("uPrevTransform");
	Uniform<glm::mat4> uCurrTransformG = Uniform<glm::mat4>("uCurrTransform");
	Uniform<glm::vec4> uAtlasDataG = Uniform<glm::vec4>("uAtlasData");
	Uniform<glm::vec2> uVelG = Uniform<glm::vec2>("uVel");
	Uniform<GLfloat> uExposureTimeG = Uniform<GLfloat>("uExposureTime");
	Uniform<GLfloat> uMaxVelocityMagG = Uniform<GLfloat>("uMaxVelocityMag");
	UniformMaterial uMaterialG = UniformMaterial("uMaterial");
};
