#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include "System.h"

struct Level;

class SystemManager
{
public:
	static SystemManager &getInstance();

	SystemManager(const SystemManager &) = delete;
	SystemManager(const SystemManager &&) = delete;
	SystemManager &operator= (const SystemManager &) = delete;
	SystemManager &operator= (const SystemManager &&) = delete;
	void input(double _currentTime, double _timeDelta);
	void update(double _currentTime, double _timeDelta);
	void render();
	template<typename SystemType, typename ...Args>
	SystemType *addSystem(Args&& ..._args);
	template<typename SystemType>
	SystemType *getSystem();
	void init();
	void setLevel(const std::shared_ptr<Level> &_level);
	std::shared_ptr<Level> getLevel();

private:
	bool m_initialized = false;
	std::vector<BaseSystem *> m_systems;
	std::shared_ptr<Level> m_level;

	SystemManager() = default;
	~SystemManager();
};

template<typename SystemType, typename ...Args>
inline SystemType *SystemManager::addSystem(Args&& ..._args)
{
	SystemType *system = new SystemType(std::forward<Args>(_args)...);
	m_systems.push_back(system);
	return system;
}

template<typename SystemType>
inline SystemType *SystemManager::getSystem()
{
	auto it = std::find_if(m_systems.begin(), m_systems.end(), [](BaseSystem *_system) {return _system->getTypeIdOfDerived() == SystemType::getTypeId(); });
	return it != m_systems.end() ? static_cast<SystemType *>(*it) : nullptr;
}
