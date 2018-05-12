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
#include "IGameLogic.h"
#include "Window.h"
#include "EntityComponentSystem\SystemManager.h"
#include "UserInput.h"

double Engine::currentTime = 0.0;
Engine* Engine::instance = nullptr;

Engine::Engine(const std::string &_title, IGameLogic & _gameLogic)
	: window(Window::createWindow(_title)), gameLogic(_gameLogic), userInput(UserInput::getInstance()), title(_title), systemManager(SystemManager::getInstance())
{
	assert(!instance);
	instance = this;
}

Engine::~Engine()
{
}

void Engine::start()
{
	showFps = SettingsManager::getInstance().getBoolSetting("misc", "show_fps", true);
	window->init();
	window->addInputListener(&userInput);
	systemManager.addSystem<MovementSystem>();
	systemManager.addSystem<MovementPathSystem>();
	systemManager.addSystem<RotationSystem>();
	systemManager.addSystem<PerpetualRotationSystem>();
	systemManager.addSystem<RenderSystem>(window);
	systemManager.addSystem<GrabbingSystem>(window);
	systemManager.addSystem<SoundSystem>();
	systemManager.init();
	gameLogic.init();

	gameLoop();
}

void Engine::shutdown()
{
	shouldShutdown = true;
}

double Engine::getCurrentTime()
{
	return currentTime;
}

Engine* Engine::getInstance()
{
	return instance;
}

Window* Engine::getWindow()
{
	return window.get();
}

void Engine::runLater(const std::function<void()> &function)
{
	std::lock_guard<std::mutex> lock(instance->mutex);
	instance->functionQueue.push_back(function);
}

bool Engine::isFunctionQueueEmpty()
{
	std::lock_guard<std::mutex> lock(instance->mutex);
	return instance->functionQueue.empty();
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
	//make sure we can use this clock
	assert(std::chrono::high_resolution_clock::is_steady);

	lastFpsMeasure = currentTime = lastFrame = glfwGetTime();
	while (!shouldShutdown && !window->shouldClose())
	{
		currentTime = glfwGetTime();
		double delta = currentTime - lastFrame;

		input(currentTime, delta);
		update(currentTime, delta);
		render();

		lastFrame = currentTime;
	}
	window->destroy();
}

void Engine::input(const double &_currentTime, const double &_timeDelta)
{
	userInput.input();
	systemManager.input(_currentTime, _timeDelta);
	gameLogic.input(_currentTime, _timeDelta);
}

void Engine::update(const double &_currentTime, const double &_timeDelta)
{
	systemManager.update(_currentTime, _timeDelta);
	gameLogic.update(_currentTime, _timeDelta);

	std::lock_guard<std::mutex> lock(mutex);
	if (!functionQueue.empty())
	{
		for (auto function : functionQueue)
		{
			function();
		}
		functionQueue.clear();
	}
}

void Engine::render()
{
	double difference = currentTime - lastFpsMeasure;
	if (difference > 1.0 && showFps->get())
	{
		fps /= difference;
		window->setTitle(title + " - " + std::to_string(fps) + " FPS " + std::to_string(1.0/fps*1000.0) + " ms");
		lastFpsMeasure = currentTime;
		fps = 0;
	}
	++fps;
	systemManager.render();
	gameLogic.render();
	window->update();
}
