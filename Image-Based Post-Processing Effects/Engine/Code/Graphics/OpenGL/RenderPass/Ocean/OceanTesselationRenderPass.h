#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct RenderData;
struct Level;
class TileRing;

class OceanTesselationRenderPass : public RenderPass
{
public:
	explicit OceanTesselationRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, GLuint _displacementTexture, GLuint _normalTexture, TileRing **_tileRings, bool _wireframe, RenderPass ** _previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> oceanTesselationShader;

	Uniform<glm::mat4> uViewProjectionWT = Uniform<glm::mat4>("uViewProjection");
	Uniform<glm::mat4> uProjectionWT = Uniform<glm::mat4>("uProjection");
	Uniform<glm::mat4> uViewWT = Uniform<glm::mat4>("uView");
	Uniform<glm::vec3> uCamPosWT = Uniform<glm::vec3>("uCamPos");
	Uniform<glm::vec2> uTexCoordShiftWT = Uniform<glm::vec2>("uTexCoordShift");
	Uniform<bool> uUseEnvironmentWT = Uniform<bool>("uUseEnvironment");
	Uniform<float> uWaterLevelWT = Uniform<float>("uVerticalDisplacement");
	Uniform<glm::vec3> uLightDirWT = Uniform<glm::vec3>("uLightDir");
	Uniform<glm::vec3> uLightColorWT = Uniform<glm::vec3>("uLightColor");
	Uniform<GLfloat> uTileSizeWT = Uniform<GLfloat>("uTileSize");
	Uniform<glm::vec3> uViewDirWT = Uniform<glm::vec3>("uViewDir");
	Uniform<glm::vec2> uScreenSizeWT = Uniform<glm::vec2>("uScreenSize");
	Uniform<GLint> uTesselatedTriWidthWT = Uniform<GLint>("uTesselatedTriWidth");
	Uniform<GLfloat> uTexCoordScaleWT = Uniform<GLfloat>("uTexCoordScale");
	Uniform<GLfloat> uDisplacementScaleWT = Uniform<GLfloat>("uDisplacementScale");
};
