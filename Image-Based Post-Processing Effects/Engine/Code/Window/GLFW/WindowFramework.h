#pragma once
#include <string>
#include <vector>
#define GLFW_INCLUDE_NONE
#include <GLFW\glfw3.h>
#include <memory>
#include "Settings.h"

class IInputListener;
class IWindowResizeListener;
struct Gamepad;

class WindowFramework
{
	friend void windowSizeCallback(GLFWwindow *window, int width, int height);
	friend void curserPosCallback(GLFWwindow *window, double xPos, double yPos);
	friend void curserEnterCallback(GLFWwindow *window, int entered);
	friend void scrollCallback(GLFWwindow *window, double xOffset, double yOffset);
	friend void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
	friend void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
	friend void charCallback(GLFWwindow *window, unsigned int codepoint);
	friend void joystickCallback(int joystickId, int event);

public:
	static std::shared_ptr<WindowFramework> createWindowFramework(const std::string &_title, unsigned int _width, unsigned int _height, bool _vsync, const WindowMode &_windowMode = WindowMode::WINDOWED);
	
	WindowFramework(const WindowFramework &) = delete;
	WindowFramework(const WindowFramework &&) = delete;
	WindowFramework &operator= (const WindowFramework &) = delete;
	WindowFramework &operator= (const WindowFramework &&) = delete;
	~WindowFramework();
	void init();
	void update();
	void addInputListener(IInputListener *listener);
	void removeInputListener(IInputListener *listener);
	void addResizeListener(IWindowResizeListener *listener);
	void removeResizeListener(IWindowResizeListener *listener);
	unsigned int getWidth() const;
	unsigned int getHeight() const;
	size_t getSelectedResolutionIndex() const;
	std::pair<unsigned int, unsigned int> getSelectedResolution() const;
	std::vector<std::pair<unsigned int, unsigned int>> getSupportedResolutions();
	void destroyWindow();
	bool shouldClose();
	void setTitle(const std::string &_title);
	void setVsync(bool _vsync);
	void setShouldClose(bool _shouldClose);
	void setWindowMode(const WindowMode &_windowMode);
	void setResolution(const std::pair<unsigned int, unsigned int> &_resolution);
	void setWindowModeAndResolution(const WindowMode &_windowMode, const std::pair<unsigned int, unsigned int> &_resolution);
	void setIcon(size_t count, const char *sizes, unsigned char **pixelData);

	WindowMode getWindowMode() const;
	std::pair<unsigned int, unsigned int> getNativeResolution();

private:
	std::string title;
	std::vector<IWindowResizeListener*> resizeListeners;
	std::vector<IInputListener*> inputListeners;
	std::pair<unsigned int, unsigned int> *currentResolution;
	std::pair<unsigned int, unsigned int> nativeResolution;
	std::pair<unsigned int, unsigned int> selectedResolution;
	size_t selectedResolutionIndex;
	bool vsync;
	unsigned int antialiasing;
	GLFWwindow *window = nullptr;
	std::vector<std::pair<unsigned int, unsigned int>> supportedResolutions;
	WindowMode windowMode;
	std::vector<Gamepad> gamepads;

	explicit WindowFramework(const std::string &_title, unsigned int _width, unsigned int _height, bool _vsync, const WindowMode &_windowMode = WindowMode::WINDOWED);
	void updateSelectedResolutionIndex();
};