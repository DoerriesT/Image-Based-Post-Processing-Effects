#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct RenderData;
class Scene;
struct Effects;
struct Level;

class AmbientLightRenderPass : public RenderPass
{
public:
	explicit AmbientLightRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const RenderData &_renderData, const std::shared_ptr<Level> &_level, const Effects &_effects, GLuint ssaoTexture, GLuint _brdfLUT, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> m_ambientLightShader;
	std::shared_ptr<Mesh> m_fullscreenTriangle;

	UniformDirectionalLight m_uDirectionalLight = UniformDirectionalLight("uDirectionalLight");

	Uniform<GLboolean> m_uOddFrame = Uniform<GLboolean>("uOddFrame");
	Uniform<glm::mat4> m_uInverseProjection = Uniform<glm::mat4>("uInverseProjection");
	Uniform<glm::mat4> m_uInverseView = Uniform<glm::mat4>("uInverseView");

	Uniform<glm::vec3> m_uVolumeOrigin = Uniform<glm::vec3>("uVolumeOrigin");
	Uniform<glm::ivec3> m_uVolumeDimensions = Uniform<glm::ivec3>("uVolumeDimensions");
	Uniform<GLfloat> m_uSpacing = Uniform<GLfloat>("uSpacing");

	void createUniforms();
};
