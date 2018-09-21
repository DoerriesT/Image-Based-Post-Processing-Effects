#pragma once
#include <string>
#include <glm\mat4x4.hpp>
#include "Settings.h"
#include "GLFW\IWindowResizeListener.h"

class IInputListener;
class WindowFramework;

class Window : public IWindowResizeListener
{
public:
	static const float DEFAULT_FOV;
	static const float NEAR_PLANE;
	static const float FAR_PLANE;
	
	static std::shared_ptr<Window> createWindow(const std::string &_title);

	Window(const Window &) = delete;
	Window(const Window &&) = delete;
	Window &operator= (const Window &) = delete;
	Window &operator= (const Window &&) = delete;
	void init();
	bool shouldClose();
	void destroy();
	void update();
	void addInputListener(IInputListener *listener);
	void removeInputListener(IInputListener *listener);
	void addResizeListener(IWindowResizeListener *listener);
	void removeResizeListener(IWindowResizeListener *listener);
	unsigned int getWidth() const;
	unsigned int getHeight() const;
	const glm::mat4 &getProjectionMatrix() const;
	size_t getSelectedResolutionIndex() const;
	void setTitle(const std::string &_title);
	void setIcon(size_t count, const char *sizes, unsigned char **pixelData);
	void grabMouse(bool _grabMouse);
	void setFieldOfView(float _fov);
	void resetFieldOfView();
	float getFieldOfView() const;
	std::pair<unsigned int, unsigned int> getSelectedResolution() const;
	std::vector<std::pair<unsigned int, unsigned int>> getSupportedResolutions();
	void onResize(unsigned int width, unsigned int height) override;

private:
	std::shared_ptr<Setting<bool>> vsync;
	std::shared_ptr<Setting<int>> windowWidth;
	std::shared_ptr<Setting<int>> windowHeight;
	std::shared_ptr<Setting<int>> windowMode;

	std::shared_ptr<WindowFramework> windowFramework;
	std::vector<IWindowResizeListener *> resizeListeners;
	glm::mat4 projectionMatrix;
	float fieldOfView;

	explicit Window(const std::string &_title);
};