#include "SystemManager.h"

SystemManager::~SystemManager()
{
	for (BaseSystem *baseSystem : m_systems)
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
	for (BaseSystem *system : m_systems)
	{
		system->input(_currentTime, _timeDelta);
	}
}

void SystemManager::update(double _currentTime, double _timeDelta)
{
	for (BaseSystem *system : m_systems)
	{
		system->update(_currentTime, _timeDelta);
	}
}

void SystemManager::render()
{
	for (BaseSystem *system : m_systems)
	{
		system->render();
	}
}

void SystemManager::init()
{
	if (!m_initialized)
	{
		m_initialized = true;
		for (BaseSystem *system : m_systems)
		{
			system->init();
		}
	}
}

void SystemManager::setLevel(const std::shared_ptr<Level> &_level)
{
	m_level = _level;
}

std::shared_ptr<Level> SystemManager::getLevel()
{
	return m_level;
}
