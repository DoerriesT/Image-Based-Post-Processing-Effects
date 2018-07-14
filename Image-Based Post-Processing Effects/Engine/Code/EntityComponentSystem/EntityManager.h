#pragma once
#include <cstdint>
#include <unordered_map>
#include <set>
#include <vector>
#include <cassert>
#include "Component.h"
#include "Utilities\ContainerUtility.h"
#include "IOnComponentAddedListener.h"
#include "IOnComponentRemovedListener.h"
#include "IOnEntityCreatedListener.h"
#include "IOnEntityDestructionListener.h"


class EntityManager;

struct Entity
{
	friend class EntityManager;

public:
	typedef std::uint64_t Id;
	typedef std::uint64_t Version;

	Entity() = default;
	Entity(const Id &_id, const Id &_version);
	~Entity() = default;

	const Id id;
	const Version version;
};

class EntityManager
{
public:
	static EntityManager &getInstance();

	EntityManager(const EntityManager &) = delete;
	EntityManager(const EntityManager &&) = delete;
	EntityManager &operator= (const EntityManager &) = delete;
	EntityManager &operator= (const EntityManager &&) = delete;
	const Entity *createEntity();
	template<typename ComponentType, typename ...Args>
	ComponentType *addComponent(const Entity *_entity, Args&& ..._args);
	template<typename ComponentType>
	void removeComponent(const Entity *_entity, bool _notify = true);
	void removeComponentFamily(const Entity *_entity, std::uint64_t _familyId, bool _notify = true);
	template<typename ComponentType>
	ComponentType *getComponent(const Entity *_entity);
	void destroyEntity(const Entity *_entity);
	std::uint64_t getComponentBitField(const Entity *_entity);
	std::unordered_map<std::uint64_t, BaseComponent *> getComponentMap(const Entity *_entity);
	void addOnEntityCreatedListener(IOnEntityCreatedListener *_listener);
	void addOnEntityDestructionListener(IOnEntityDestructionListener *_listener);
	void addOnComponentAddedListener(IOnComponentAddedListener *_listener);
	void addOnComponentRemovedListener(IOnComponentRemovedListener *_listener);
	void removeOnEntityCreatedListener(IOnEntityCreatedListener *_listener);
	void removeOnEntityDestructionListener(IOnEntityDestructionListener *_listener);
	void removeOnComponentAddedListener(IOnComponentAddedListener *_listener);
	void removeOnComponentRemovedListener(IOnComponentRemovedListener *_listener);

private:
	std::uint64_t nextFreeId;
	std::vector<std::uint64_t> freeIds;
	std::set<Entity> entities;
	std::unordered_map<Entity::Id, std::uint64_t> entityIdToComponentBitFieldMap;
	std::unordered_map<Entity::Id, std::uint64_t> entityIdToFamilyBitFieldMap;
	std::unordered_map<Entity::Id, std::unordered_map<std::uint64_t, BaseComponent *>> entityIdToComponentMap;
	std::unordered_map<Entity::Id, std::unordered_map<std::uint64_t, BaseComponent *>> entityIdToFamilyMap;
	std::unordered_map<Entity::Id, Entity::Version> entityIdVersionMap;
	std::vector<IOnEntityCreatedListener *> onEntityCreatedListeners;
	std::vector<IOnEntityDestructionListener *> onEntityDestructionListeners;
	std::vector<IOnComponentAddedListener *> onComponentAddedListeners;
	std::vector<IOnComponentRemovedListener *> onComponentRemovedListeners;

	EntityManager() = default;
	~EntityManager() = default;
	bool validateEntity(const Entity *_entity);
};

template<typename ComponentType, typename ...Args>
ComponentType *EntityManager::addComponent(const Entity *_entity, Args&& ..._args)
{
	assert(validateEntity(_entity));

	const Entity::Id &id = _entity->id;

	std::uint64_t typeId = ComponentType::getTypeId();
	std::uint64_t familyId = ComponentType::getFamilyId();

	// if a component of this type or family already exists, remove it
	if (entityIdToComponentBitFieldMap[id] & typeId)
	{
		removeComponent<ComponentType>(_entity);
	}
	else if (entityIdToFamilyBitFieldMap[id] & familyId)
	{
		removeComponentFamily(_entity, familyId);
	}

	// set the type and family bit
	entityIdToComponentBitFieldMap[id] |= typeId;
	entityIdToFamilyBitFieldMap[id] |= familyId;

	// create the new component
	ComponentType *component = new ComponentType(std::forward<Args>(_args)...);

	// add the component to type and family maps
	entityIdToComponentMap[id].emplace(typeId, component);
	entityIdToFamilyMap[id].emplace(familyId, component);

	//invoke listeners
	for (IOnComponentAddedListener *listener : onComponentAddedListeners)
	{
		listener->onComponentAdded(_entity, component);
	}

	return component;
}

template<typename ComponentType>
void EntityManager::removeComponent(const Entity *_entity, bool _notify)
{
	assert(validateEntity(_entity));

	const Entity::Id &id = _entity->id;

	std::uint64_t typeId = ComponentType::getTypeId();
	std::uint64_t familyId = ComponentType::getFamilyId();

	// only do something if the component is actually attached to the entity
	if (entityIdToComponentBitFieldMap[id] & typeId)
	{
		// fetch the component pointer
		BaseComponent *component = entityIdToComponentMap[id][typeId];
		assert(component);

		// remove type and family bits from their map
		entityIdToComponentBitFieldMap[id] ^= typeId;
		entityIdToFamilyBitFieldMap[id] ^= familyId;

		// invoke listeners
		if (_notify)
		{
			for (IOnComponentRemovedListener *listener : onComponentRemovedListeners)
			{
				listener->onComponentRemoved(_entity, component);
			}
		}

		// delete pointer and remove component from maps
		delete component;
		ContainerUtility::remove(entityIdToComponentMap[id], typeId);
		ContainerUtility::remove(entityIdToFamilyMap[id], familyId);
	}
}

template<typename ComponentType>
inline ComponentType *EntityManager::getComponent(const Entity *_entity)
{
	assert(_entity);

	ComponentType *component = nullptr;

	//assert that there is a bitmap attached to the entity
	assert(ContainerUtility::contains(entityIdToComponentBitFieldMap, _entity->id));

	// check if the component is attached to the entity
	if((entityIdToComponentBitFieldMap[_entity->id] & ComponentType::getTypeId()) == ComponentType::getTypeId())
	{
		assert(ContainerUtility::contains(entityIdToComponentMap[_entity->id], ComponentType::getTypeId()));
		component = static_cast<ComponentType *>(entityIdToComponentMap[_entity->id][ComponentType::getTypeId()]);
	}

	return component;
}
