#pragma once
#include "GLRenderer.h"
#include "EnvironmentRenderer.h"
#include "Window\GLFW\IWindowResizeListener.h"

class Camera;
class Scene;
class Window;
class Mesh;
class EnvironmentProbe;
struct Level;
struct Effects;

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
	void bake(const Scene &_scene, const std::shared_ptr<Level> &_level, unsigned int _bounces, bool _reflections, bool _irradianceVolume);
	std::shared_ptr<Texture> render(const AtmosphereParams &_params);
	void blitToScreen();
	GLuint getFinishedFrameTexture();
	void onResize(unsigned int _width, unsigned int _height) override;

private:
	GLRenderer renderer;
	EnvironmentRenderer environmentRenderer;
	std::shared_ptr<ShaderProgram> blitShader;
	std::shared_ptr<Window> window;

	std::shared_ptr<Mesh> fullscreenTriangle;

	// blit shader uniform
	GLint uScreenTextureBlit;
	GLint uRedToWhiteBlit;
	GLint uScaleBlit;
	GLint uNormalModeBlit;
	GLint uInvViewMatrixBlit;
	GLint uPowerBlit;
	GLint uPowerValueBlit;

	unsigned int frame;
};

