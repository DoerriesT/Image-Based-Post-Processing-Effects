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
	std::shared_ptr<ShaderProgram> simplePostEffectsShader;
	std::shared_ptr<Mesh> fullscreenTriangle;

	Uniform<GLfloat> uTimeS = Uniform<GLfloat>("uTime");
	Uniform<GLfloat> uFilmGrainStrengthS = Uniform<GLfloat>("uFilmGrainStrength");
	Uniform<GLboolean> uVignetteS = Uniform<GLboolean>("uVignette");
	Uniform<GLboolean> uFilmGrainS = Uniform<GLboolean>("uFilmGrain");
	Uniform<GLboolean> uChromaticAberrationS = Uniform<GLboolean>("uChromaticAberration");
	Uniform<GLfloat> uChromAbOffsetMultiplierS = Uniform<GLfloat>("uChromAbOffsetMultiplier");

};
