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
	void input(const double &_currentTime, const double &_timeDelta);
	void update(const double &_currentTime, const double &_timeDelta);
	void render();
	template<typename SystemType, typename ...Args>
	SystemType *addSystem(Args&& ..._args);
	template<typename SystemType>
	SystemType *getSystem();
	void init();
	void setLevel(const std::shared_ptr<Level> &_level);
	std::shared_ptr<Level> getLevel();

private:
	bool initialized = false;
	std::vector<BaseSystem *> systems;
	std::shared_ptr<Level> level;

	SystemManager() = default;
	~SystemManager();
};

template<typename SystemType, typename ...Args>
inline SystemType *SystemManager::addSystem(Args&& ..._args)
{
	SystemType *system = new SystemType(std::forward<Args>(_args)...);
	systems.push_back(system);
	return system;
}

template<typename SystemType>
inline SystemType *SystemManager::getSystem()
{
	auto it = std::find_if(systems.begin(), systems.end(), [](BaseSystem *_system) {return _system->getTypeIdOfDerived() == SystemType::getTypeId(); });
	return it != systems.end() ? static_cast<SystemType *>(*it) : nullptr;
}
