#include <glm\gtc\matrix_transform.hpp>
#include <algorithm>
#include "Window.h"
#include "GLFW\IInputListener.h"
#include "GLFW\WindowFramework.h"

const float Window::DEFAULT_FOV = 59.0f;
const float Window::NEAR_PLANE = 0.1f;
const float Window::FAR_PLANE = 300.0f;

Window::Window(const std::string &_title)
	:m_vsync(SettingsManager::getInstance().getBoolSetting("graphics", "vsync", false)),
	m_windowWidth(SettingsManager::getInstance().getIntSetting("graphics", "window_width", 1080)),
	m_windowHeight(SettingsManager::getInstance().getIntSetting("graphics", "window_height", 720)),
	m_windowMode(SettingsManager::getInstance().getIntSetting("graphics", "window_mode", 0)),
	m_windowFramework(WindowFramework::createWindowFramework(_title, m_windowWidth->get(), m_windowHeight->get(), m_vsync->get(), (WindowMode)m_windowMode->get()))
{
	m_fieldOfView = DEFAULT_FOV;
	m_windowFramework->addResizeListener(this);
	m_projectionMatrix = glm::perspective(glm::radians(m_fieldOfView), static_cast<float>(m_windowWidth->get()) / static_cast<float>(m_windowHeight->get()), NEAR_PLANE, FAR_PLANE);
};

std::shared_ptr<Window> Window::createWindow(const std::string &_title)
{
	return std::shared_ptr<Window>(new Window(_title));
}

void Window::init()
{
	m_vsync->addListener([&](bool _value) 
	{ 
		m_windowFramework->setVsync(_value); 
	});

	m_windowWidth->addListener([&](int _value) 
	{ 
		m_windowFramework->setResolution(std::make_pair<unsigned int, unsigned int>((unsigned int)_value, 0u));
		unsigned int w = getWidth();
		unsigned int h = getHeight();
		onResize(w, h);
	});

	m_windowHeight->addListener([&](int _value) 
	{
		m_windowFramework->setResolution(std::make_pair<unsigned int, unsigned int>(0u, (unsigned int)_value)); 
		unsigned int w = getWidth();
		unsigned int h = getHeight();
		onResize(w, h);
	});

	m_windowMode->addListener([&](int _value) 
	{ 
		m_windowFramework->setWindowMode((WindowMode)_value); 
		unsigned int w = getWidth();
		unsigned int h = getHeight();
		onResize(w, h);
	});

	SettingsManager::getInstance().saveToIni();

	m_windowFramework->init();
}

bool Window::shouldClose()
{
	return m_windowFramework->shouldClose();
}

void Window::destroy()
{
	m_windowFramework->destroyWindow();
}

void Window::update()
{
	m_windowFramework->update();
}

void Window::addInputListener(IInputListener *listener)
{
	m_windowFramework->addInputListener(listener);
}

void Window::removeInputListener(IInputListener *listener)
{
	m_windowFramework->removeInputListener(listener);
}

void Window::addResizeListener(IWindowResizeListener *listener)
{
	m_resizeListeners.push_back(listener);
}

void Window::removeResizeListener(IWindowResizeListener *listener)
{
	m_resizeListeners.erase(std::remove(m_resizeListeners.begin(), m_resizeListeners.end(), listener), m_resizeListeners.end());
}

unsigned int Window::getWidth() const
{
	return m_windowFramework->getWidth();
}

unsigned int Window::getHeight() const
{
	return m_windowFramework->getHeight();
}

const glm::mat4 &Window::getProjectionMatrix() const
{
	return m_projectionMatrix;
}

size_t Window::getSelectedResolutionIndex() const
{
	return m_windowFramework->getSelectedResolutionIndex();
}

void Window::setTitle(const std::string &_title)
{
	m_windowFramework->setTitle(_title);
}

void Window::setIcon(size_t count, const char *sizes, unsigned char **pixelData)
{
	m_windowFramework->setIcon(count, sizes, pixelData);
}

void Window::grabMouse(bool _grabMouse)
{
	m_windowFramework->grabMouse(_grabMouse);
}

void Window::setFieldOfView(float _fov)
{
	m_fieldOfView = _fov;
	m_projectionMatrix = glm::perspective(glm::radians(m_fieldOfView), static_cast<float>(getWidth()) / static_cast<float>(getHeight()), NEAR_PLANE, FAR_PLANE);
}

void Window::resetFieldOfView()
{
	m_fieldOfView = DEFAULT_FOV;
}

float Window::getFieldOfView() const
{
	return m_fieldOfView;
}

std::pair<unsigned int, unsigned int> Window::getSelectedResolution() const
{
	return m_windowFramework->getSelectedResolution();
}

std::vector<std::pair<unsigned int, unsigned int>> Window::getSupportedResolutions()
{
	return m_windowFramework->getSupportedResolutions();
}

void Window::onResize(unsigned int width, unsigned int height)
{
	if (width != 0 && height != 0)
	{
		m_projectionMatrix = glm::perspective(glm::radians(m_fieldOfView), static_cast<float>(width) / static_cast<float>(height), NEAR_PLANE, FAR_PLANE);
		for (IWindowResizeListener *listener : m_resizeListeners)
		{
			listener->onResize(width, height);
		}
	}
}
