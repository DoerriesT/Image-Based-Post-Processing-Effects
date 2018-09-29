#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct Effects;

class SMAABlendWeightRenderPass : public RenderPass
{
public:
	explicit SMAABlendWeightRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const Effects &_effects, GLuint _edgesTexture, bool _temporal, bool _currentSample, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> blendWeightShader;
	std::shared_ptr<Mesh> fullscreenTriangle;

	Uniform<glm::vec4> uResolutionSMAAB = Uniform<glm::vec4>("uResolution");
	Uniform<GLboolean> uTemporalSampleSMAAB = Uniform<GLboolean>("uTemporalSample");
	Uniform<GLboolean> uTemporalAASMAAB = Uniform<GLboolean>("uTemporalAA");

};
