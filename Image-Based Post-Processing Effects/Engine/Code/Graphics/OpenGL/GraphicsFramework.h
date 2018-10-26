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
	GLRenderer m_renderer;
	EnvironmentRenderer m_environmentRenderer;
	std::shared_ptr<ShaderProgram> m_blitShader;
	std::shared_ptr<Window> m_window;

	std::shared_ptr<Mesh> m_fullscreenTriangle;

	// blit shader uniform
	GLint m_uScreenTexture;
	GLint m_uRedToWhite;
	GLint m_uScale;
	GLint m_uNormalMode;
	GLint m_uInvViewMatrix;
	GLint m_uPower;
	GLint m_uPowerValue;

	unsigned int m_frame;
};

