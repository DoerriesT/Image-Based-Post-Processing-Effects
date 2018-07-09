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
#include "Window.h"
#include "EntityComponentSystem\SystemManager.h"
#include "Input\UserInput.h"

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
	systemManager.addSystem<PhysicsSystem>();
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

double Engine::getTime()
{
	return instance->time;
}

double Engine::getTimeDelta()
{
	return instance->timeDelta;
}

double Engine::getFps()
{
	return 1.0 / instance->timeDelta;
}

Engine *Engine::getInstance()
{
	return instance;
}

Window *Engine::getWindow()
{
	return window.get();
}

void Engine::runLater(std::function<void()> function)
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
	lastFpsMeasure = time = lastFrame = glfwGetTime();
	while (!shouldShutdown && !window->shouldClose())
	{
		time = glfwGetTime();
		timeDelta = time - lastFrame;

		input(time, timeDelta);
		update(time, timeDelta);
		render();

		lastFrame = time;
	}
	window->destroy();
}

void Engine::input(double _currentTime, double _timeDelta)
{
	userInput.input();
	systemManager.input(_currentTime, _timeDelta);
	gameLogic.input(_currentTime, _timeDelta);
}

void Engine::update(double _currentTime, double _timeDelta)
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
	double difference = time - lastFpsMeasure;
	if (difference > 1.0 && showFps->get())
	{
		fps /= difference;
		window->setTitle(title + " - " + std::to_string(fps) + " FPS " + std::to_string(1.0 / fps*1000.0) + " ms");
		lastFpsMeasure = time;
		fps = 0;
	}
	++fps;
	systemManager.render();
	gameLogic.render();
	window->update();
}
