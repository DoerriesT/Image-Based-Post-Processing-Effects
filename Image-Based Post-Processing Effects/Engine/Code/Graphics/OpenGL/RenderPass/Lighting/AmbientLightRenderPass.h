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

class AmbientLightRenderPass : public RenderPass
{
public:
	explicit AmbientLightRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const Effects &_effects, const GBuffer &_gbuffer, GLuint _brdfLUT, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> environmentLightPassShader;
	std::shared_ptr<Mesh> fullscreenTriangle;

	Uniform<glm::mat4> uProjectionE = Uniform<glm::mat4>("uProjection");
	Uniform<glm::mat4> uInverseProjectionE = Uniform<glm::mat4>("uInverseProjection");
	Uniform<glm::mat4> uInverseViewE = Uniform<glm::mat4>("uInverseView");
	Uniform<glm::mat4> uReProjectionE = Uniform<glm::mat4>("uReProjection");

	Uniform<glm::vec3> uVolumeOrigin = Uniform<glm::vec3>("uVolumeOrigin");
	Uniform<glm::ivec3> uVolumeDimensions = Uniform<glm::ivec3>("uVolumeDimensions");
	Uniform<GLfloat> uSpacing = Uniform<GLfloat>("uSpacing");

	void createUniforms();
};
