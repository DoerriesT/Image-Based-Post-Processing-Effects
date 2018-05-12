#include "SystemManager.h"

SystemManager::~SystemManager()
{
	for (BaseSystem *baseSystem : systems)
	{
			delete baseSystem;
	}
}

SystemManager &SystemManager::getInstance()
{
	static SystemManager instance;
	return instance;
}

void SystemManager::input(double _currentTime, double _timeDelta)
{
	for (BaseSystem *system : systems)
	{
		system->input(_currentTime, _timeDelta);
	}
}

void SystemManager::update(double _currentTime, double _timeDelta)
{
	for (BaseSystem *system : systems)
	{
		system->update(_currentTime, _timeDelta);
	}
}

void SystemManager::render()
{
	for (BaseSystem *system : systems)
	{
		system->render();
	}
}

void SystemManager::init()
{
	if (!initialized)
	{
		initialized = true;
		for (BaseSystem *system : systems)
		{
			system->init();
		}
	}
}

void SystemManager::setLevel(const std::shared_ptr<Level> &_level)
{
	level = _level;
}

std::shared_ptr<Level> SystemManager::getLevel()
{
	return level;
}
