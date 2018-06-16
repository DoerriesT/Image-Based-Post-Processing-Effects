#pragma once
#include <glm\vec3.hpp>
#include <glm\mat4x4.hpp>
#include <memory>
#include <glad\glad.h>
#include ".\..\..\Graphics\AxisAlignedBoundingBox.h"

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
	void renderShadows(const RenderData &_renderData, const Scene &_scene, const std::shared_ptr<Level> &_level, const Effects &_effects, const std::shared_ptr<Camera> &_camera);

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

	float splits[4];

	void render(const glm::mat4 &_viewProjectionMatrix, const Scene &_scene);
	void createFboAttachments(const std::pair<unsigned int, unsigned int> &_resolution);
	void blur(GLuint _textureToBlur);
	void deleteAttachments();
	void deleteFbos();
	void blit(GLuint targetTexture);
	glm::mat4 calculateLightViewProjection(const RenderData & _renderData, const std::shared_ptr<Camera>& _camera, const glm::vec3 & _lightDir, float _nearPlane, float _farPlane);
	glm::mat4 calculateLightViewProjection(const RenderData &_renderData, const AxisAlignedBoundingBox &_sceneAABB, const glm::vec3 &_lightDir, float _nearPlane, float _farPlane, bool _useAABB);
	AxisAlignedBoundingBox calculateSceneAABB(const Scene &_scene);
};