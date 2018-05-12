#pragma once
#include <glm\vec3.hpp>
#include <glm\mat4x4.hpp>
#include <memory>
#include <glad\glad.h>

class ShaderProgram;
class Camera;
class Scene;
class Mesh;
struct Level;
struct Effects;
struct RenderData;


class ShadowRenderer
{
public:
	explicit ShadowRenderer();
	ShadowRenderer(const ShadowRenderer &) = delete;
	ShadowRenderer(const ShadowRenderer &&) = delete;
	ShadowRenderer &operator= (const ShadowRenderer &) = delete;
	ShadowRenderer &operator= (const ShadowRenderer &&) = delete;
	~ShadowRenderer();
	void init();
	void renderShadows(const RenderData &_renderData, const Scene &_scene, const std::shared_ptr<Level> &_level, const Effects &_effects);

private:
	std::shared_ptr<ShaderProgram> shadowShader;
	std::shared_ptr<ShaderProgram> shadowBlurShader;
	std::shared_ptr<ShaderProgram> blitShader;

	std::shared_ptr<Mesh> fullscreenTriangle;

	// fbo
	GLuint shadowFbo;
	GLuint shadowTextureA;
	GLuint shadowTextureB;
	GLuint depthRenderBuffer;

	GLint uModelViewProjectionMatrix;
	GLint uModelMatrix;
	GLint uCamPos;
	GLint uShadowMap;
	GLint uDirection;
	GLint uWidth;
	GLint uHeight;

	// blit
	GLuint uScreenTextureB;

	void render(const glm::mat4 &_viewProjectionMatrix, const Scene &_scene);
	void createFboAttachments(const std::pair<unsigned int, unsigned int> &_resolution);
	void blur(const GLuint &_textureToBlur);
	void deleteAttachments();
	void deleteFbos();
	void blit(GLuint targetTexture);
};