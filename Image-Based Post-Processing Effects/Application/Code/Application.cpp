#include "Application.h"
#include "Level.h"
#include <EntityComponentSystem\SystemManager.h>

namespace App
{
	Application::Application()
		:cameraController()
	{
	}

	void Application::init()
	{
		level = loadLevel();
		SystemManager::getInstance().setLevel(level);
		cameraController.setCamera(level->cameras[level->activeCameraIndex]);
	}

	void Application::input(double currentTime, double timeDelta)
	{
		cameraController.input(currentTime, timeDelta);
	}

	void Application::update(double currentTime, double timeDelta)
	{
	}

	void Application::render()
	{
	}
}