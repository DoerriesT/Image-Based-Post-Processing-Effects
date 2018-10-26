#pragma once
#include "Graphics\OpenGL\RenderPass\RenderPass.h"
#include "Graphics\OpenGL\ShaderProgram.h"
#include "Graphics\OpenGL\Uniform.h"
#include "Graphics\Mesh.h"

struct Effects;

class SimplePostEffectsRenderPass : public RenderPass
{
public:
	explicit SimplePostEffectsRenderPass(GLuint _fbo, unsigned int _width, unsigned int _height);
	void render(const Effects &_effects, GLuint _inputTexture, GLenum _drawBuffer, RenderPass **_previousRenderPass = nullptr);

private:
	std::shared_ptr<ShaderProgram> m_simplePostEffectsShader;
	std::shared_ptr<Mesh> m_fullscreenTriangle;

	Uniform<GLfloat> m_uTime = Uniform<GLfloat>("uTime");
	Uniform<GLfloat> m_uFilmGrainStrength = Uniform<GLfloat>("uFilmGrainStrength");
	Uniform<GLboolean> m_uVignette = Uniform<GLboolean>("uVignette");
	Uniform<GLboolean> m_uFilmGrain = Uniform<GLboolean>("uFilmGrain");
	Uniform<GLboolean> m_uChromaticAberration = Uniform<GLboolean>("uChromaticAberration");
	Uniform<GLfloat> m_uChromAbOffsetMultiplier = Uniform<GLfloat>("uChromAbOffsetMultiplier");

};
