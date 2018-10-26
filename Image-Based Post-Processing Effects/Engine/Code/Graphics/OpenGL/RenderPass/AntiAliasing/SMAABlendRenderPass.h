#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct Effects;

class SMAABlendRenderPass : public RenderPass
{
public:
	explicit SMAABlendRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const Effects &_effects, GLuint _inputTexture, GLuint _velocityTexture, GLuint _blendWeightTexture, bool _currentSample, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> m_blendShader;
	std::shared_ptr<Mesh> m_fullscreenTriangle;

	Uniform<glm::vec4> m_uResolution = Uniform<glm::vec4>("uResolution");

};
