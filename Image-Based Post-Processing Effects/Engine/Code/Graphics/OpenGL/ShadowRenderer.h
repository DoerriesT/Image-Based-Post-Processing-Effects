#pragma once
#include <glm\vec3.hpp>
#include <glm\mat4x4.hpp>
#include <memory>
#include <glad\glad.h>
#include "Graphics\AxisAlignedBoundingBox.h"
#include "Graphics\Lights.h"

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
	void setCascadeSkipOptimization(bool _enabled);

private:
	std::shared_ptr<ShaderProgram> shadowShader;
	bool cascadeSkipOptimization;
	unsigned int frameCounter;

	// fbo
	GLuint shadowFbo;

	GLint uModelViewProjectionMatrix;

	void render(const glm::mat4 &_viewProjectionMatrix, const Scene &_scene);
	void deleteFbos();
	glm::mat4 calculateLightViewProjection(const RenderData &_renderData, const AxisAlignedBoundingBox &_sceneAABB, const glm::vec3 &_lightDir, float _nearPlane, float _farPlane, bool _useAABB);
	AxisAlignedBoundingBox calculateSceneAABB(const Scene &_scene);
};