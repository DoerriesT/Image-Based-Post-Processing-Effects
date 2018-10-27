#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct RenderData;
class Scene;
struct Effects;
struct Level;

class GTAOTemporalDenoiseRenderPass : public RenderPass
{
public:
	explicit GTAOTemporalDenoiseRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const RenderData &_renderData, const Effects &_effects, GLuint _velocityTexture, GLuint *_ssaoTextures, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> m_gtaoDenoiseShader;
	std::shared_ptr<Mesh> m_fullscreenTriangle;

	Uniform<GLfloat> m_uFrameTime = Uniform<GLfloat>("uFrameTime");
	Uniform<glm::mat4> m_uInvProjection = Uniform<glm::mat4>("uInvProjection");
	Uniform<glm::mat4> m_uInvView = Uniform<glm::mat4>("uInvView");
	Uniform<glm::mat4> m_uPrevInvProjection = Uniform<glm::mat4>("uPrevInvProjection");
	Uniform<glm::mat4> m_uPrevInvView = Uniform<glm::mat4>("uPrevInvView");
};
