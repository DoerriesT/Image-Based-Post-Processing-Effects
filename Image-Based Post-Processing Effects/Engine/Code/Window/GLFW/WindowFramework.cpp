#include <algorithm>
#include <iostream>
#include "WindowFramework.h"
#include "IWindowResizeListener.h"
#include "IInputListener.h"
#include "Utilities\ContainerUtility.h"
#include "Input\Gamepad.h"
#include <cassert>

void windowSizeCallback(GLFWwindow *window, int width, int height);

void curserPosCallback(GLFWwindow *window, double xPos, double yPos);

void curserEnterCallback(GLFWwindow *window, int entered);

void scrollCallback(GLFWwindow *window, double xOffset, double yOffset);

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

void charCallback(GLFWwindow *window, unsigned int codepoint);

void joystickCallback(int joystickId, int event);

WindowFramework::WindowFramework(const std::string &_title, unsigned int _width, unsigned int _height, bool _vsync, const WindowMode &_windowMode)
	:title(_title), selectedResolution(std::make_pair(_width, _height)), currentResolution(&selectedResolution), vsync(_vsync), windowMode(_windowMode), gamepads(16)
{
}

void WindowFramework::updateSelectedResolutionIndex()
{
	assert(!supportedResolutions.empty());

	for (size_t i = 0; i < supportedResolutions.size(); ++i)
	{
		const unsigned int w = supportedResolutions[i].first;
		const unsigned int h = supportedResolutions[i].second;
		if (w == selectedResolution.first && h == selectedResolution.second)
		{
			selectedResolutionIndex = i;
			return;
		}
	}
	//assert(false);
}

std::shared_ptr<WindowFramework> WindowFramework::createWindowFramework(const std::string &_title, unsigned int _width, unsigned int _height, bool _vsync, const WindowMode &_windowMode)
{
	return std::shared_ptr<WindowFramework>(new WindowFramework(_title, _width, _height, _vsync, _windowMode));
}

WindowFramework::~WindowFramework()
{
	glfwTerminate();
}

void WindowFramework::init()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

#ifdef _DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif // _DEBUG


	glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);
	glfwWindowHint(GLFW_ALPHA_BITS, 0);
	glfwWindowHint(GLFW_DEPTH_BITS, 0);
	glfwWindowHint(GLFW_STENCIL_BITS, 0);

	window = glfwCreateWindow(currentResolution->first, currentResolution->second, title.c_str(), NULL, NULL);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(window);

	if (vsync)
	{
		// Enable v-sync
		glfwSwapInterval(1);
	}
	else
	{
		glfwSwapInterval(0);
	}

	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSetFramebufferSizeCallback(window, windowSizeCallback);
	glfwSetCursorPosCallback(window, curserPosCallback);
	glfwSetCursorEnterCallback(window, curserEnterCallback);
	glfwSetScrollCallback(window, scrollCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetCharCallback(window, charCallback);
	glfwSetJoystickCallback(joystickCallback);

	glfwSetWindowUserPointer(window, this);


	// create list of supported resolutions

	int vidModeCount;
	const GLFWvidmode *vidModes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &vidModeCount);
	bool addToList = true;

	for (int i = 0; i < vidModeCount; ++i)
	{
		int width = vidModes[i].width;
		int height = vidModes[i].height;

		// we only support resolutions of 800x600 and higher (also they must be evenly divisible by 8)
		if (width < 800 || height < 600 || width % 8 != 0 || height % 8 != 0)
		{
			continue;
		}
		// make sure we do not already have this resolution in our list
		addToList = true;
		for (std::pair<unsigned int, unsigned int> &resolution : supportedResolutions)
		{
			if (resolution.first == width && resolution.second == height)
			{
				addToList = false;
			}
		}
		if (addToList)
		{
			supportedResolutions.push_back(std::make_pair(width, height));
		}
	}

	const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	nativeResolution = std::make_pair<unsigned int, unsigned int>((unsigned int)mode->width, (unsigned int)mode->height);

	for (std::pair<unsigned int, unsigned int> &resolution : supportedResolutions)
	{
		if (resolution.first > nativeResolution.first || resolution.second > nativeResolution.second)
		{
			ContainerUtility::remove(supportedResolutions, resolution);
		}
	}

	setWindowMode(windowMode);
	updateSelectedResolutionIndex();
}

void WindowFramework::update()
{
	glfwSwapBuffers(window);
	glfwPollEvents();

	const float *axisValues;
	const unsigned char *buttonValues;
	int axisCount;
	int buttonCount;

	for (int i = 0; i < 16; ++i)
	{
		bool connected = glfwJoystickPresent(i);

		axisValues = glfwGetJoystickAxes(i, &axisCount);
		buttonValues = glfwGetJoystickButtons(i, &buttonCount);

		if (connected && axisCount == 6 && buttonCount == 14)
		{
			/*
			buttons:
			0 = A
			1 = B
			2 = X
			3 = Y
			4 = LB
			5 = RB
			6 = back
			7 = start
			8 = left stick
			9 = right stick
			10 = up
			11 = right
			12 = down
			13 = left

			axes:
			0 = left x
			1 = left y
			2 = right x
			3 = right y
			4 = LT
			5 = RT
			*/

			gamepads[i].id = i;
			gamepads[i].buttonA = buttonValues[0];
			gamepads[i].buttonB = buttonValues[1];
			gamepads[i].buttonX = buttonValues[2];
			gamepads[i].buttonY = buttonValues[3];
			gamepads[i].leftButton = buttonValues[4];
			gamepads[i].rightButton = buttonValues[5];
			gamepads[i].backButton = buttonValues[6];
			gamepads[i].startButton = buttonValues[7];
			gamepads[i].leftStick = buttonValues[8];
			gamepads[i].rightStick = buttonValues[9];
			gamepads[i].dPadUp = buttonValues[10];
			gamepads[i].dPadRight = buttonValues[11];
			gamepads[i].dPadDown = buttonValues[12];
			gamepads[i].dPadLeft = buttonValues[13];
			gamepads[i].leftStickX = axisValues[0];
			gamepads[i].leftStickY = axisValues[1];
			gamepads[i].rightStickX = axisValues[2];
			gamepads[i].rightStickY = axisValues[3];
			gamepads[i].leftTrigger = axisValues[4];
			gamepads[i].rightTrigger = axisValues[5];
		}
		else
		{
			gamepads[i].id = -1;
		}
	}
}

void WindowFramework::addInputListener(IInputListener *listener)
{
	inputListeners.push_back(listener);
	listener->gamepadUpdate(&gamepads);
}

void WindowFramework::removeInputListener(IInputListener *listener)
{
	inputListeners.erase(std::remove(inputListeners.begin(), inputListeners.end(), listener), inputListeners.end());
}

void WindowFramework::addResizeListener(IWindowResizeListener *listener)
{
	resizeListeners.push_back(listener);
}

void WindowFramework::removeResizeListener(IWindowResizeListener *listener)
{
	resizeListeners.erase(std::remove(resizeListeners.begin(), resizeListeners.end(), listener), resizeListeners.end());
}

unsigned int WindowFramework::getWidth() const
{
	return currentResolution->first;
}

unsigned int WindowFramework::getHeight() const
{
	return currentResolution->second;
}

size_t WindowFramework::getSelectedResolutionIndex() const
{
	return selectedResolutionIndex;
}

std::pair<unsigned int, unsigned int> WindowFramework::getSelectedResolution() const
{
	return selectedResolution;
}

std::vector<std::pair<unsigned int, unsigned int>> WindowFramework::getSupportedResolutions()
{
	return supportedResolutions;
}

void WindowFramework::destroyWindow()
{
	glfwDestroyWindow(window);
}

bool WindowFramework::shouldClose()
{
	return glfwWindowShouldClose(window);
}

void WindowFramework::setTitle(const std::string &_title)
{
	title = _title;
	glfwSetWindowTitle(window, title.c_str());
}

void WindowFramework::setVsync(bool _vsync)
{
	if (vsync != _vsync)
	{
		vsync = _vsync;
		glfwSwapInterval(vsync);
	}
}

void WindowFramework::setShouldClose(bool _shouldClose)
{
	glfwSetWindowShouldClose(window, _shouldClose);
}

void WindowFramework::setWindowMode(const WindowMode &_windowMode)
{
	GLFWmonitor *monitor = glfwGetPrimaryMonitor();

	if (_windowMode == WindowMode::WINDOWED)
	{
		bool notify = selectedResolution != *currentResolution;
		currentResolution = &selectedResolution;
		// request windowed video mode

		const GLFWvidmode *vidMode = glfwGetVideoMode(monitor);
		int x = vidMode->width/2 - currentResolution->first/2;
		int y = vidMode->height/2 - currentResolution->second/2;
		if (y < 32)
		{
			y = 32;
		}
		glfwSetWindowMonitor(window, NULL, x, y, currentResolution->first, currentResolution->second, GLFW_DONT_CARE);
		// notify listeners if resolution changed
		if (notify)
		{
			for (IWindowResizeListener *listener : resizeListeners)
			{
				listener->onResize(currentResolution->first, currentResolution->second);
			}
		}
	}
	else if (_windowMode == WindowMode::BORDERLESS_FULLSCREEN/* && windowMode != WindowMode::BORDERLESS_FULLSCREEN*/)
	{
		if (windowMode == WindowMode::FULLSCREEN)
		{
			glfwSetWindowMonitor(window, NULL, 0, 0, 1, 1, GLFW_DONT_CARE);
		}
		// request fake fullscreen window by requesting the current vidmode.
		const GLFWvidmode *vidMode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor(window, monitor, 0, 0, vidMode->width, vidMode->height, vidMode->refreshRate);

		bool notify = nativeResolution != *currentResolution;
		currentResolution = &nativeResolution;
		// notify listeners if resolution changed
		if (notify)
		{
			for (IWindowResizeListener *listener : resizeListeners)
			{
				listener->onResize(currentResolution->first, currentResolution->second);
			}
		}
	}
	else if (_windowMode == WindowMode::FULLSCREEN)
	{
		bool notify = selectedResolution != *currentResolution;
		currentResolution = &selectedResolution;

		// if we are currently in fake fullscreen set to windowed
		if (windowMode == WindowMode::BORDERLESS_FULLSCREEN)
		{
			glfwSetWindowMonitor(window, NULL, 0, 0, 1, 1, GLFW_DONT_CARE);
		}
		// request fullscreen window
		glfwSetWindowMonitor(window, monitor, 0, 0, currentResolution->first, currentResolution->second, GLFW_DONT_CARE);
		// notify listeners if resolution changed
		if (notify)
		{
			for (IWindowResizeListener *listener : resizeListeners)
			{
				listener->onResize(currentResolution->first, currentResolution->second);
			}
		}
	}
	// make sure vsync is set to the correct setting
	glfwSwapInterval(vsync);
	windowMode = _windowMode;
}

void WindowFramework::setResolution(const std::pair<unsigned int, unsigned int> &_resolution)
{
	selectedResolution.first = _resolution.first ? _resolution.first : selectedResolution.first;
	selectedResolution.second = _resolution.second ? _resolution.second : selectedResolution.second;
	updateSelectedResolutionIndex();
	setWindowMode(windowMode);
}

void WindowFramework::setWindowModeAndResolution(const WindowMode &_windowMode, const std::pair<unsigned int, unsigned int> &_resolution)
{
	selectedResolution.first = _resolution.first ? _resolution.first : selectedResolution.first;
	selectedResolution.second = _resolution.second ? _resolution.second : selectedResolution.second;
	updateSelectedResolutionIndex();
	setWindowMode(_windowMode);
}

void WindowFramework::setIcon(size_t count, const char *sizes, unsigned char **pixelData)
{
	GLFWimage *images = new GLFWimage[count];

	for (size_t i = 0; i<count; ++i)
	{
		images[i].width = sizes[i];
		images[i].height = sizes[i];
		images[i].pixels = pixelData[i];
	}
	glfwSetWindowIcon(window, (int)count, images);
	delete[] images;
}

void WindowFramework::grabMouse(bool _grabMouse)
{
	if (_grabMouse)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	else
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

WindowMode WindowFramework::getWindowMode() const
{
	return windowMode;
}

std::pair<unsigned int, unsigned int> WindowFramework::getNativeResolution()
{
	return nativeResolution;
}

// callback functions

void windowSizeCallback(GLFWwindow *window, int width, int height)
{
	// we invoke our own resize callbacks and do not propagate those invoked by glfw

	/*WindowFramework *windowFramework = static_cast<WindowFramework *>(glfwGetWindowUserPointer(window));
	for (IWindowResizeListener *listener : windowFramework->resizeListeners)
	{
		listener->onResize(width, height);
	}*/
}

void curserPosCallback(GLFWwindow *window, double xPos, double yPos)
{
	WindowFramework *windowFramework = static_cast<WindowFramework *>(glfwGetWindowUserPointer(window));
	for (IInputListener *listener : windowFramework->inputListeners)
	{
		listener->onMouseMove(xPos, yPos);
	}
}

void curserEnterCallback(GLFWwindow *window, int entered)
{
	WindowFramework *windowFramework = static_cast<WindowFramework *>(glfwGetWindowUserPointer(window));
	for (IInputListener *listener : windowFramework->inputListeners)
	{
		listener->onMouseEnter(entered);
	}
}

void scrollCallback(GLFWwindow *window, double xOffset, double yOffset)
{
	WindowFramework *windowFramework = static_cast<WindowFramework *>(glfwGetWindowUserPointer(window));
	for (IInputListener *listener : windowFramework->inputListeners)
	{
		listener->onMouseScroll(xOffset, yOffset);
	}
}

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
	WindowFramework *windowFramework = static_cast<WindowFramework *>(glfwGetWindowUserPointer(window));
	for (IInputListener *listener : windowFramework->inputListeners)
	{
		listener->onMouseButton(button, action);
	}
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	WindowFramework *windowFramework = static_cast<WindowFramework *>(glfwGetWindowUserPointer(window));
	for (IInputListener *listener : windowFramework->inputListeners)
	{
		listener->onKey(key, action);
	}
}

void charCallback(GLFWwindow *window, unsigned int codepoint)
{
	WindowFramework *windowFramework = static_cast<WindowFramework *>(glfwGetWindowUserPointer(window));
	for (IInputListener *listener : windowFramework->inputListeners)
	{
		listener->onChar(codepoint);
	}
}

void joystickCallback(int joystickId, int event)
{
	if (event == GLFW_CONNECTED)
	{

	}
	else if (event == GLFW_DISCONNECTED)
	{

	}
}
