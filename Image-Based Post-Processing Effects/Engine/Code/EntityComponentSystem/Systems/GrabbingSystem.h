#pragma once
#include <vector>
#include ".\..\IOnComponentAddedListener.h"
#include ".\..\IOnComponentRemovedListener.h"
#include ".\..\IOnEntityDestructionListener.h"
#include ".\..\System.h"
#include <memory>

class Window;
class EntityManager;

class GrabbingSystem : public System<GrabbingSystem>, IOnComponentAddedListener, IOnComponentRemovedListener, IOnEntityDestructionListener
{
public:
	explicit GrabbingSystem(std::shared_ptr<Window> _window);
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
	std::vector<const Entity *> entitiesToRemove;
	std::vector<const Entity *> entitiesToAdd;
	std::vector<std::uint64_t> validBitMaps;
	std::shared_ptr<Window> window;

	bool validate(const std::uint64_t &_bitMap);
};