#include <cassert>
#include <iostream>
#include <glad\glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW\glfw3.h>
#include "Engine.h"
#include "EntityComponentSystem\Systems\RenderSystem.h"
#include "EntityComponentSystem\Systems\MovementSystem.h"
#include "EntityComponentSystem\Systems\RotationSystem.h"
#include "EntityComponentSystem\Systems\GrabbingSystem.h"
#include "EntityComponentSystem\Systems\SoundSystem.h"
#include "EntityComponentSystem\Systems\PerpetualRotationSystem.h"
#include "EntityComponentSystem\Systems\MovementPathSystem.h"
#include "EntityComponentSystem\Systems\PhysicsSystem.h"
#include "IGameLogic.h"
#include "Window\Window.h"
#include "EntityComponentSystem\SystemManager.h"
#include "Input\UserInput.h"

Engine* Engine::s_instance = nullptr;

Engine::Engine(const std::string &_title, IGameLogic & _gameLogic)
	: m_window(Window::createWindow(_title)), 
	m_gameLogic(_gameLogic), 
	m_userInput(UserInput::getInstance()),
	m_title(_title), 
	m_systemManager(SystemManager::getInstance()),
	m_lastFrame(),
	m_time(),
	m_timeDelta(),
	m_lastFpsMeasure(),
	m_fps()
{
	assert(!s_instance);
	s_instance = this;
}

Engine::~Engine()
{
}

void Engine::start()
{
	m_showFps = SettingsManager::getInstance().getBoolSetting("misc", "show_fps", true);
	m_window->init();
	m_window->addInputListener(&m_userInput);
	m_systemManager.addSystem<MovementSystem>();
	m_systemManager.addSystem<MovementPathSystem>();
	m_systemManager.addSystem<RotationSystem>();
	m_systemManager.addSystem<PerpetualRotationSystem>();
	m_systemManager.addSystem<RenderSystem>(m_window);
	m_systemManager.addSystem<PhysicsSystem>();
	m_systemManager.addSystem<GrabbingSystem>(m_window);
	m_systemManager.addSystem<SoundSystem>();
	m_systemManager.init();
	m_gameLogic.init();

	gameLoop();
}

void Engine::shutdown()
{
	m_shouldShutdown = true;
}

double Engine::getTime()
{
	return s_instance->m_time;
}

double Engine::getTimeDelta()
{
	return s_instance->m_timeDelta;
}

double Engine::getFps()
{
	return 1.0 / s_instance->m_timeDelta;
}

Engine *Engine::getInstance()
{
	return s_instance;
}

Window *Engine::getWindow()
{
	return m_window.get();
}

void Engine::runLater(std::function<void()> function)
{
	std::lock_guard<std::mutex> lock(s_instance->m_mutex);
	s_instance->m_functionQueue.push_back(function);
}

bool Engine::isFunctionQueueEmpty()
{
	std::lock_guard<std::mutex> lock(s_instance->m_mutex);
	return s_instance->m_functionQueue.empty();
}

int Engine::getMaxAnisotropicFiltering()
{
	int aa = 1;
	if (GLAD_GL_EXT_texture_filter_anisotropic)
	{
		glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aa);
	}
	else
	{
		std::cout << "OpenGL extension GL_EXT_texture_filter_anisotropic is not supported!" << std::endl;
	}
	return aa;
}

void Engine::gameLoop()
{
	m_lastFpsMeasure = m_time = m_lastFrame = glfwGetTime();
	while (!m_shouldShutdown && !m_window->shouldClose())
	{
		m_time = glfwGetTime();
		m_timeDelta = m_time - m_lastFrame;

		input(m_time, m_timeDelta);
		update(m_time, m_timeDelta);
		render();

		m_lastFrame = m_time;
	}
	m_window->destroy();
}

void Engine::input(double _currentTime, double _timeDelta)
{
	m_userInput.input();
	m_systemManager.input(_currentTime, _timeDelta);
	m_gameLogic.input(_currentTime, _timeDelta);
}

void Engine::update(double _currentTime, double _timeDelta)
{
	m_systemManager.update(_currentTime, _timeDelta);
	m_gameLogic.update(_currentTime, _timeDelta);

	std::lock_guard<std::mutex> lock(m_mutex);
	if (!m_functionQueue.empty())
	{
		for (auto function : m_functionQueue)
		{
			function();
		}
		m_functionQueue.clear();
	}
}

void Engine::render()
{
	double difference = m_time - m_lastFpsMeasure;
	if (difference > 1.0 && m_showFps->get())
	{
		m_fps /= difference;
		m_window->setTitle(m_title + " - " + std::to_string(m_fps) + " FPS " + std::to_string(1.0 / m_fps*1000.0) + " ms");
		m_lastFpsMeasure = m_time;
		m_fps = 0;
	}
	++m_fps;
	m_systemManager.render();
	m_gameLogic.render();
	m_window->update();
}
