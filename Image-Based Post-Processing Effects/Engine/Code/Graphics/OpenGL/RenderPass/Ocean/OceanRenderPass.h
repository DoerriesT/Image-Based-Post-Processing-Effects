#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct RenderData;
struct Level;

class OceanRenderPass : public RenderPass
{
public:
	explicit OceanRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, GLuint _displacementTexture, GLuint _normalTexture, GLuint _gridVAO, GLsizei _gridSize, bool _wireframe, RenderPass ** _previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> oceanShader;

	Uniform<glm::mat4> uProjectionW = Uniform<glm::mat4>("uProjection");
	Uniform<glm::mat4> uViewW = Uniform<glm::mat4>("uView");
	Uniform<glm::vec3> uCamPosW = Uniform<glm::vec3>("uCamPos");
	Uniform<glm::vec2> uTexCoordShiftW = Uniform<glm::vec2>("uTexCoordShift");
	Uniform<bool> uUseEnvironmentW = Uniform<bool>("uUseEnvironment");
	Uniform<float> uWaterLevelW = Uniform<float>("uWaterLevel");
	Uniform<glm::vec3> uLightDirW = Uniform<glm::vec3>("uLightDir");
	Uniform<glm::vec3> uLightColorW = Uniform<glm::vec3>("uLightColor");
};
