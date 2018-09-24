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

class GTAOSpatialDenoiseRenderPass : public RenderPass
{
public:
	explicit GTAOSpatialDenoiseRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const RenderData &_renderData, const Effects &_effects, const GBuffer &_gbuffer, GLuint *_ssaoTextures, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> gtaoDenoiseShader;
	std::shared_ptr<Mesh> fullscreenTriangle;

	Uniform<GLint> uFrame = Uniform<GLint>("uFrame");

};
