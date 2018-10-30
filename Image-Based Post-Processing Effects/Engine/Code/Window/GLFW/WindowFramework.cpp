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
	:m_title(_title), m_selectedResolution(std::make_pair(_width, _height)), m_currentResolution(&m_selectedResolution), m_vsync(_vsync), m_windowMode(_windowMode), m_gamepads(16), m_selectedResolutionIndex()
{
}

void WindowFramework::updateSelectedResolutionIndex()
{
	assert(!m_supportedResolutions.empty());

	for (size_t i = 0; i < m_supportedResolutions.size(); ++i)
	{
		const unsigned int w = m_supportedResolutions[i].first;
		const unsigned int h = m_supportedResolutions[i].second;
		if (w == m_selectedResolution.first && h == m_selectedResolution.second)
		{
			m_selectedResolutionIndex = i;
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

	m_window = glfwCreateWindow(m_currentResolution->first, m_currentResolution->second, m_title.c_str(), NULL, NULL);
	if (m_window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(m_window);

	if (m_vsync)
	{
		// Enable v-sync
		glfwSwapInterval(1);
	}
	else
	{
		glfwSwapInterval(0);
	}

	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSetFramebufferSizeCallback(m_window, windowSizeCallback);
	glfwSetCursorPosCallback(m_window, curserPosCallback);
	glfwSetCursorEnterCallback(m_window, curserEnterCallback);
	glfwSetScrollCallback(m_window, scrollCallback);
	glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
	glfwSetKeyCallback(m_window, keyCallback);
	glfwSetCharCallback(m_window, charCallback);
	glfwSetJoystickCallback(joystickCallback);

	glfwSetWindowUserPointer(m_window, this);


	// create list of supported resolutions

	int vidModeCount;
	const GLFWvidmode *vidModes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &vidModeCount);
	bool addToList = true;

	for (int i = 0; i < vidModeCount; ++i)
	{
		int width = vidModes[i].width;
		int height = vidModes[i].height;

		// we only support resolutions of 800x600 and higher
		if (width < 800 || height < 600)
		{
			continue;
		}
		// make sure we do not already have this resolution in our list
		addToList = true;
		for (std::pair<unsigned int, unsigned int> &resolution : m_supportedResolutions)
		{
			if (resolution.first == width && resolution.second == height)
			{
				addToList = false;
			}
		}
		if (addToList)
		{
			m_supportedResolutions.push_back(std::make_pair(width, height));
		}
	}

	const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	m_nativeResolution = std::make_pair<unsigned int, unsigned int>((unsigned int)mode->width, (unsigned int)mode->height);

	for (std::pair<unsigned int, unsigned int> &resolution : m_supportedResolutions)
	{
		if (resolution.first > m_nativeResolution.first || resolution.second > m_nativeResolution.second)
		{
			ContainerUtility::remove(m_supportedResolutions, resolution);
		}
	}

	setWindowMode(m_windowMode);
	updateSelectedResolutionIndex();
}

void WindowFramework::update()
{
	glfwSwapBuffers(m_window);
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

			m_gamepads[i].m_id = i;
			m_gamepads[i].m_buttonA = buttonValues[0];
			m_gamepads[i].m_buttonB = buttonValues[1];
			m_gamepads[i].m_buttonX = buttonValues[2];
			m_gamepads[i].m_buttonY = buttonValues[3];
			m_gamepads[i].m_leftButton = buttonValues[4];
			m_gamepads[i].m_rightButton = buttonValues[5];
			m_gamepads[i].m_backButton = buttonValues[6];
			m_gamepads[i].m_startButton = buttonValues[7];
			m_gamepads[i].m_leftStick = buttonValues[8];
			m_gamepads[i].m_rightStick = buttonValues[9];
			m_gamepads[i].m_dPadUp = buttonValues[10];
			m_gamepads[i].m_dPadRight = buttonValues[11];
			m_gamepads[i].m_dPadDown = buttonValues[12];
			m_gamepads[i].m_dPadLeft = buttonValues[13];
			m_gamepads[i].m_leftStickX = axisValues[0];
			m_gamepads[i].m_leftStickY = axisValues[1];
			m_gamepads[i].m_rightStickX = axisValues[2];
			m_gamepads[i].m_rightStickY = axisValues[3];
			m_gamepads[i].m_leftTrigger = axisValues[4];
			m_gamepads[i].m_rightTrigger = axisValues[5];
		}
		else
		{
			m_gamepads[i].m_id = -1;
		}
	}
}

void WindowFramework::addInputListener(IInputListener *listener)
{
	m_inputListeners.push_back(listener);
	listener->gamepadUpdate(&m_gamepads);
}

void WindowFramework::removeInputListener(IInputListener *listener)
{
	m_inputListeners.erase(std::remove(m_inputListeners.begin(), m_inputListeners.end(), listener), m_inputListeners.end());
}

void WindowFramework::addResizeListener(IWindowResizeListener *listener)
{
	m_resizeListeners.push_back(listener);
}

void WindowFramework::removeResizeListener(IWindowResizeListener *listener)
{
	m_resizeListeners.erase(std::remove(m_resizeListeners.begin(), m_resizeListeners.end(), listener), m_resizeListeners.end());
}

unsigned int WindowFramework::getWidth() const
{
	return m_currentResolution->first;
}

unsigned int WindowFramework::getHeight() const
{
	return m_currentResolution->second;
}

size_t WindowFramework::getSelectedResolutionIndex() const
{
	return m_selectedResolutionIndex;
}

std::pair<unsigned int, unsigned int> WindowFramework::getSelectedResolution() const
{
	return m_selectedResolution;
}

std::vector<std::pair<unsigned int, unsigned int>> WindowFramework::getSupportedResolutions()
{
	return m_supportedResolutions;
}

void WindowFramework::destroyWindow()
{
	glfwDestroyWindow(m_window);
}

bool WindowFramework::shouldClose()
{
	return glfwWindowShouldClose(m_window);
}

void WindowFramework::setTitle(const std::string &_title)
{
	m_title = _title;
	glfwSetWindowTitle(m_window, m_title.c_str());
}

void WindowFramework::setVsync(bool _vsync)
{
	if (m_vsync != _vsync)
	{
		m_vsync = _vsync;
		glfwSwapInterval(m_vsync);
	}
}

void WindowFramework::setShouldClose(bool _shouldClose)
{
	glfwSetWindowShouldClose(m_window, _shouldClose);
}

void WindowFramework::setWindowMode(const WindowMode &_windowMode)
{
	GLFWmonitor *monitor = glfwGetPrimaryMonitor();

	if (_windowMode == WindowMode::WINDOWED)
	{
		bool notify = m_selectedResolution != *m_currentResolution;
		m_currentResolution = &m_selectedResolution;
		// request windowed video mode

		const GLFWvidmode *vidMode = glfwGetVideoMode(monitor);
		int x = vidMode->width/2 - m_currentResolution->first/2;
		int y = vidMode->height/2 - m_currentResolution->second/2;
		if (y < 32)
		{
			y = 32;
		}
		glfwSetWindowMonitor(m_window, NULL, x, y, m_currentResolution->first, m_currentResolution->second, GLFW_DONT_CARE);
		// notify listeners if resolution changed
		if (notify)
		{
			for (IWindowResizeListener *listener : m_resizeListeners)
			{
				listener->onResize(m_currentResolution->first, m_currentResolution->second);
			}
		}
	}
	else if (_windowMode == WindowMode::BORDERLESS_FULLSCREEN/* && windowMode != WindowMode::BORDERLESS_FULLSCREEN*/)
	{
		if (m_windowMode == WindowMode::FULLSCREEN)
		{
			glfwSetWindowMonitor(m_window, NULL, 0, 0, 1, 1, GLFW_DONT_CARE);
		}
		// request fake fullscreen window by requesting the current vidmode.
		const GLFWvidmode *vidMode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor(m_window, monitor, 0, 0, vidMode->width, vidMode->height, vidMode->refreshRate);

		bool notify = m_nativeResolution != *m_currentResolution;
		m_currentResolution = &m_nativeResolution;
		// notify listeners if resolution changed
		if (notify)
		{
			for (IWindowResizeListener *listener : m_resizeListeners)
			{
				listener->onResize(m_currentResolution->first, m_currentResolution->second);
			}
		}
	}
	else if (_windowMode == WindowMode::FULLSCREEN)
	{
		bool notify = m_selectedResolution != *m_currentResolution;
		m_currentResolution = &m_selectedResolution;

		// if we are currently in fake fullscreen set to windowed
		if (m_windowMode == WindowMode::BORDERLESS_FULLSCREEN)
		{
			glfwSetWindowMonitor(m_window, NULL, 0, 0, 1, 1, GLFW_DONT_CARE);
		}
		// request fullscreen window
		glfwSetWindowMonitor(m_window, monitor, 0, 0, m_currentResolution->first, m_currentResolution->second, GLFW_DONT_CARE);
		// notify listeners if resolution changed
		if (notify)
		{
			for (IWindowResizeListener *listener : m_resizeListeners)
			{
				listener->onResize(m_currentResolution->first, m_currentResolution->second);
			}
		}
	}
	// make sure vsync is set to the correct setting
	glfwSwapInterval(m_vsync);
	m_windowMode = _windowMode;
}

void WindowFramework::setResolution(const std::pair<unsigned int, unsigned int> &_resolution)
{
	m_selectedResolution.first = _resolution.first ? _resolution.first : m_selectedResolution.first;
	m_selectedResolution.second = _resolution.second ? _resolution.second : m_selectedResolution.second;
	updateSelectedResolutionIndex();
	setWindowMode(m_windowMode);
}

void WindowFramework::setWindowModeAndResolution(const WindowMode &_windowMode, const std::pair<unsigned int, unsigned int> &_resolution)
{
	m_selectedResolution.first = _resolution.first ? _resolution.first : m_selectedResolution.first;
	m_selectedResolution.second = _resolution.second ? _resolution.second : m_selectedResolution.second;
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
	glfwSetWindowIcon(m_window, (int)count, images);
	delete[] images;
}

void WindowFramework::grabMouse(bool _grabMouse)
{
	if (_grabMouse)
	{
		glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	else
	{
		glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

WindowMode WindowFramework::getWindowMode() const
{
	return m_windowMode;
}

std::pair<unsigned int, unsigned int> WindowFramework::getNativeResolution()
{
	return m_nativeResolution;
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
	for (IInputListener *listener : windowFramework->m_inputListeners)
	{
		listener->onMouseMove(xPos, yPos);
	}
}

void curserEnterCallback(GLFWwindow *window, int entered)
{
	WindowFramework *windowFramework = static_cast<WindowFramework *>(glfwGetWindowUserPointer(window));
	for (IInputListener *listener : windowFramework->m_inputListeners)
	{
		listener->onMouseEnter(entered);
	}
}

void scrollCallback(GLFWwindow *window, double xOffset, double yOffset)
{
	WindowFramework *windowFramework = static_cast<WindowFramework *>(glfwGetWindowUserPointer(window));
	for (IInputListener *listener : windowFramework->m_inputListeners)
	{
		listener->onMouseScroll(xOffset, yOffset);
	}
}

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
	WindowFramework *windowFramework = static_cast<WindowFramework *>(glfwGetWindowUserPointer(window));
	for (IInputListener *listener : windowFramework->m_inputListeners)
	{
		listener->onMouseButton(button, action);
	}
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	WindowFramework *windowFramework = static_cast<WindowFramework *>(glfwGetWindowUserPointer(window));
	for (IInputListener *listener : windowFramework->m_inputListeners)
	{
		listener->onKey(key, action);
	}
}

void charCallback(GLFWwindow *window, unsigned int codepoint)
{
	WindowFramework *windowFramework = static_cast<WindowFramework *>(glfwGetWindowUserPointer(window));
	for (IInputListener *listener : windowFramework->m_inputListeners)
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
