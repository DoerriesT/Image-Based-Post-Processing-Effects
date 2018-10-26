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
	void input(double _currentTime, double _timeDelta) override;
	void update(double _currentTime, double _timeDelta) override;
	void render() override;
	void onComponentAdded(const Entity *_entity, BaseComponent *_addedComponent) override;
	void onComponentRemoved(const Entity *_entity, BaseComponent *_removedComponent) override;
	void onDestruction(const Entity *_entity) override;

private:
	EntityManager &m_entityManager;
	std::vector<const Entity *> m_managedEntities;
	std::vector<const Entity *> m_entitiesToRemove;
	std::vector<const Entity *> m_entitiesToAdd;
	std::vector<std::uint64_t> m_validBitMaps;
	std::shared_ptr<Window> m_window;

	bool validate(std::uint64_t _bitMap);
};