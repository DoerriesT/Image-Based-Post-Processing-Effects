#pragma once
#include "SceneRenderer.h"
#include "PostProcessRenderer.h"
#include "ShadowRenderer.h"
#include "EnvironmentRenderer.h"
#include ".\..\Window\IWindowResizeListener.h"

class Camera;
class Scene;
class Window;
class Mesh;
class EnvironmentProbe;
struct Level;
struct Effects;
enum class ShadowQuality;

class GraphicsFramework: public IWindowResizeListener
{
public:
	explicit GraphicsFramework(std::shared_ptr<Window> _window);
	GraphicsFramework(const GraphicsFramework &) = delete;
	GraphicsFramework(const GraphicsFramework &&) = delete;
	GraphicsFramework &operator= (const GraphicsFramework &) = delete;
	GraphicsFramework &operator= (const GraphicsFramework &&) = delete;
	void init();
	void render(const std::shared_ptr<Camera> &_camera, const Scene &_scene, const std::shared_ptr<Level> &_level, const Effects &_effects);
	void render(const std::shared_ptr<EnvironmentProbe> &_environmentProbe, const Scene &_scene, const std::shared_ptr<Level> &_level, const Effects &_effects);
	std::shared_ptr<Texture> render(const AtmosphereParams &_params);
	void blitToScreen();
	void setShadowQuality(const ShadowQuality &_shadowQuality);
	GLuint getFinishedFrameTexture();
	void onResize(unsigned int _width, unsigned int _height) override;

private:
	SceneRenderer sceneRenderer;
	PostProcessRenderer postProcessRenderer;
	ShadowRenderer shadowRenderer;
	EnvironmentRenderer environmentRenderer;
	std::shared_ptr<ShaderProgram> blitShader;
	std::shared_ptr<Window> window;

	std::shared_ptr<Mesh> fullscreenTriangle;

	// blit shader uniform
	GLint uScreenTextureBlit;
};

