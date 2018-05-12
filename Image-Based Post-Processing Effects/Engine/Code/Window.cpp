#include <glm\gtc\matrix_transform.hpp>
#include <algorithm>
#include "Window.h"
#include "Framework\Window\IInputListener.h"
#include "Framework\Window\WindowFramework.h"

const float Window::DEFAULT_FOV = 60.0f;
const float Window::NEAR_PLANE = 0.1f;
const float Window::FAR_PLANE = 3000.0f;

Window::Window(const std::string &_title)
	:vsync(SettingsManager::getInstance().getBoolSetting("graphics", "vsync", false)),
	windowWidth(SettingsManager::getInstance().getIntSetting("graphics", "window_width", 1080)),
	windowHeight(SettingsManager::getInstance().getIntSetting("graphics", "window_height", 720)),
	windowMode(SettingsManager::getInstance().getIntSetting("graphics", "window_mode", 0)),
	windowFramework(WindowFramework::createWindowFramework(_title, windowWidth->get(), windowHeight->get(), vsync->get(), (WindowMode)windowMode->get()))
{
	fieldOfView = DEFAULT_FOV;
	windowFramework->addResizeListener(this);
	projectionMatrix = glm::perspective(glm::radians(fieldOfView), static_cast<float>(windowWidth->get()) / static_cast<float>(windowHeight->get()), NEAR_PLANE, FAR_PLANE);
};

std::shared_ptr<Window> Window::createWindow(const std::string &_title)
{
	return std::shared_ptr<Window>(new Window(_title));
}

void Window::init()
{
	vsync->addListener([&](const bool &_value) 
	{ 
		windowFramework->setVsync(_value); 
	});

	windowWidth->addListener([&](const int &_value) 
	{ 
		windowFramework->setResolution(std::make_pair<unsigned int, unsigned int>((unsigned int)_value, 0u));
		unsigned int w = getWidth();
		unsigned int h = getHeight();
		onResize(w, h);
	});

	windowHeight->addListener([&](const int &_value) 
	{
		windowFramework->setResolution(std::make_pair<unsigned int, unsigned int>(0u, (unsigned int)_value)); 
		unsigned int w = getWidth();
		unsigned int h = getHeight();
		onResize(w, h);
	});

	windowMode->addListener([&](const int &_value) 
	{ 
		windowFramework->setWindowMode((WindowMode)_value); 
		unsigned int w = getWidth();
		unsigned int h = getHeight();
		onResize(w, h);
	});

	SettingsManager::getInstance().saveToIni();

	windowFramework->init();
}

bool Window::shouldClose()
{
	return windowFramework->shouldClose();
}

void Window::destroy()
{
	windowFramework->destroyWindow();
}

void Window::update()
{
	windowFramework->update();
}

void Window::addInputListener(IInputListener *listener)
{
	windowFramework->addInputListener(listener);
}

void Window::removeInputListener(IInputListener *listener)
{
	windowFramework->removeInputListener(listener);
}

void Window::addResizeListener(IWindowResizeListener *listener)
{
	resizeListeners.push_back(listener);
}

void Window::removeResizeListener(IWindowResizeListener *listener)
{
	resizeListeners.erase(std::remove(resizeListeners.begin(), resizeListeners.end(), listener), resizeListeners.end());
}

unsigned int Window::getWidth() const
{
	return windowFramework->getWidth();
}

unsigned int Window::getHeight() const
{
	return windowFramework->getHeight();
}

const glm::mat4 &Window::getProjectionMatrix() const
{
	return projectionMatrix;
}

void Window::setTitle(const std::string &_title)
{
	windowFramework->setTitle(_title);
}

void Window::setIcon(size_t count, const char *sizes, unsigned char **pixelData)
{
	windowFramework->setIcon(count, sizes, pixelData);
}

void Window::setFieldOfView(const float &_fov)
{
	fieldOfView = _fov;
	projectionMatrix = glm::perspective(glm::radians(fieldOfView), static_cast<float>(getWidth()) / static_cast<float>(getHeight()), NEAR_PLANE, FAR_PLANE);
}

void Window::resetFieldOfView()
{
	fieldOfView = DEFAULT_FOV;
}

float Window::getFieldOfView() const
{
	return fieldOfView;
}

std::pair<unsigned int, unsigned int> Window::getSelectedResolution() const
{
	return windowFramework->getSelectedResolution();
}

std::vector<std::pair<unsigned int, unsigned int>> Window::getSupportedResolutions()
{
	return windowFramework->getSupportedResolutions();
}

void Window::onResize(const unsigned int &width, const unsigned int &height)
{
	if (width != 0 && height != 0)
	{
		projectionMatrix = glm::perspective(glm::radians(fieldOfView), static_cast<float>(width) / static_cast<float>(height), NEAR_PLANE, FAR_PLANE);
		for (IWindowResizeListener *listener : resizeListeners)
		{
			listener->onResize(width, height);
		}
	}
}
