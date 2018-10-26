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
	std::shared_ptr<ShaderProgram> m_oceanTesselationShader;
	std::shared_ptr<Texture> m_perlinNoiseTexture;

	Uniform<glm::mat4> m_uViewProjection = Uniform<glm::mat4>("uViewProjection");
	Uniform<glm::mat4> m_uProjection = Uniform<glm::mat4>("uProjection");
	Uniform<glm::mat4> m_uView = Uniform<glm::mat4>("uView");
	Uniform<glm::vec3> m_uCamPos = Uniform<glm::vec3>("uCamPos");
	Uniform<glm::vec2> m_uTexCoordShift = Uniform<glm::vec2>("uTexCoordShift");
	Uniform<bool> m_uUseEnvironment = Uniform<bool>("uUseEnvironment");
	Uniform<float> m_uWaterLevel = Uniform<float>("uVerticalDisplacement");
	Uniform<glm::vec3> m_uLightDir = Uniform<glm::vec3>("uLightDir");
	Uniform<glm::vec3> m_uLightColor = Uniform<glm::vec3>("uLightColor");
	Uniform<GLfloat> m_uTileSize = Uniform<GLfloat>("uTileSize");
	Uniform<glm::vec3> m_uViewDir = Uniform<glm::vec3>("uViewDir");
	Uniform<glm::vec2> m_uScreenSize = Uniform<glm::vec2>("uScreenSize");
	Uniform<GLint> m_uTesselatedTriWidth = Uniform<GLint>("uTesselatedTriWidth");
	Uniform<GLfloat> m_uTexCoordScale = Uniform<GLfloat>("uTexCoordScale");
	Uniform<GLfloat> m_uDisplacementScale = Uniform<GLfloat>("uDisplacementScale");
	Uniform<glm::vec2> m_uPerlinMovement = Uniform<glm::vec2>("uPerlinMovement");
};
