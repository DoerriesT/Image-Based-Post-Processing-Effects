#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct Effects;

class SMAATemporalResolveRenderPass : public RenderPass
{
public:
	explicit SMAATemporalResolveRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const Effects &_effects, GLuint *_temporalTextures, GLuint _velocityTexture, bool _currentTexture, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> temporalResolveShader;
	std::shared_ptr<Mesh> fullscreenTriangle;

	Uniform<glm::vec4> uResolutionSMAAR = Uniform<glm::vec4>("uResolution");

};
