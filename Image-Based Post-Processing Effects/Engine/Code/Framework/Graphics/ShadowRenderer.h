#pragma once
#include <glm\vec3.hpp>
#include <glm\mat4x4.hpp>
#include <memory>
#include <glad\glad.h>
#include ".\..\..\Graphics\AxisAlignedBoundingBox.h"
#include ".\..\..\Graphics\Lights.h"

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

	// fbo
	GLuint shadowFbo;

	GLint uModelViewProjectionMatrix[6];

	void render(const glm::mat4 *_viewProjectionMatrix, unsigned int _count, const Scene &_scene);
	void deleteFbos();
	glm::mat4 calculateLightViewProjection(const RenderData &_renderData, const AxisAlignedBoundingBox &_sceneAABB, const glm::vec3 &_lightDir, float _nearPlane, float _farPlane, bool _useAABB);
	AxisAlignedBoundingBox calculateSceneAABB(const Scene &_scene);
};