#pragma once
#include <vector>
#include <memory>
#include ".\..\IOnComponentAddedListener.h"
#include ".\..\IOnComponentRemovedListener.h"
#include ".\..\IOnEntityDestructionListener.h"
#include ".\..\System.h"
#include "Settings.h"

class SoundFramework;
class Window;
class Camera;
class EntityManager;

class SoundSystem : public System<SoundSystem>, IOnComponentAddedListener, IOnComponentRemovedListener, IOnEntityDestructionListener
{
public:
	explicit SoundSystem();
	~SoundSystem();
	void init() override;
	void input(const double &_currentTime, const double &_timeDelta) override;
	void update(const double &_currentTime, const double &_timeDelta) override;
	void render() override;
	void onComponentAdded(const Entity *_entity, BaseComponent *_addedComponent) override;
	void onComponentRemoved(const Entity *_entity, BaseComponent *_removedComponent) override;
	void onDestruction(const Entity *_entity) override;

private:
	EntityManager &entityManager;
	std::vector<const Entity *> managedEntities;
	std::vector<std::uint64_t> validBitMaps;
	SoundFramework *soundFramework;

	std::shared_ptr<Setting<double>> masterVolume;
	std::shared_ptr<Setting<double>> musicVolume;
	std::shared_ptr<Setting<double>> effectVolume;
	std::shared_ptr<Setting<double>> uiVolume;

	bool validate(const std::uint64_t &_bitMap);
};
