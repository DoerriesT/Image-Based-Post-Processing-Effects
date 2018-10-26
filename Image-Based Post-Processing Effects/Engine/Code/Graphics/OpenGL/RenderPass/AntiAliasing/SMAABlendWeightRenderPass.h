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
	std::shared_ptr<ShaderProgram> m_blendWeightShader;
	std::shared_ptr<Mesh> m_fullscreenTriangle;

	Uniform<glm::vec4> m_uResolution = Uniform<glm::vec4>("uResolution");
	Uniform<GLboolean> m_uTemporalSample = Uniform<GLboolean>("uTemporalSample");
	Uniform<GLboolean> m_uTemporalAA = Uniform<GLboolean>("uTemporalAA");

};
