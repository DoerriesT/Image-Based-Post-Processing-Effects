#include <algorithm>
#include "EntityManager.h"

Entity::Entity(const Id &_id, const Id &_version)
	: id(_id), version(_version)
{
}

EntityManager &EntityManager::getInstance()
{
	static EntityManager instance;
	return instance;
}

const Entity *EntityManager::createEntity()
{
	std::uint64_t id;
	if (freeIds.empty())
	{
		id = nextFreeId++;
	}
	else
	{
		id = freeIds.back();
		freeIds.pop_back();
	}
	Entity *entity = new Entity(id, entityIdVersionMap[id]);
	entityIdToComponentBitFieldMap[id] = 0;
	entityIdToComponentMap[id].clear();
	entityIdToFamilyBitFieldMap[id] = 0;
	entityIdToFamilyMap[id].clear();

	//invoke listeners
	for (IOnEntityCreatedListener *listener : onEntityCreatedListeners)
	{
		listener->onEntityCreated(entity);
	}

	return entity;
}


void EntityManager::removeComponentFamily(const Entity *_entity, std::uint64_t _familyId, bool _notify)
{
	assert(validateEntity(_entity));

	const Entity::Id &id = _entity->id;

	// only do something if a component of this family is actually attached to the entity
	if (entityIdToFamilyBitFieldMap[id] & _familyId)
	{
		// fetch the component pointer
		BaseComponent *component = entityIdToFamilyMap[id][_familyId];
		assert(component);

		std::uint64_t typeId = component->getTypeIdOfDerived();

		// remove type and family bits from their map
		entityIdToComponentBitFieldMap[id] ^= typeId;
		entityIdToFamilyBitFieldMap[id] ^= _familyId;

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
		remove(entityIdToComponentMap[id], typeId);
		remove(entityIdToFamilyMap[id], _familyId);
	}
}


void EntityManager::destroyEntity(const Entity *_entity)
{
	assert(validateEntity(_entity));

	const Entity::Id &id = _entity->id;

	//invoke listeners
	for (IOnEntityDestructionListener *listener : onEntityDestructionListeners)
	{
		listener->onDestruction(_entity);
	}

	for (std::uint64_t i = 0, familyId = 1; i <= MAX_FAMILY_ID; ++i, familyId = 1ui64 << i)
	{
		removeComponentFamily(_entity, familyId, false);
	}

	// bitmaps should already be zero
	assert(entityIdToComponentBitFieldMap[id] == 0 && entityIdToFamilyBitFieldMap[id] == 0);
	// reset bitmaps
	// entityIdToComponentBitFieldMap[id] = 0;
	// entityIdToFamilyBitFieldMap[id] = 0;
	// increase version
	++entityIdVersionMap[id];
	freeIds.push_back(id);

	delete _entity;
}

std::uint64_t EntityManager::getComponentBitField(const Entity *_entity)
{
	assert(validateEntity(_entity));
	return entityIdToComponentBitFieldMap[_entity->id];
}

std::unordered_map<std::uint64_t, BaseComponent *> EntityManager::getComponentMap(const Entity *_entity)
{
	assert(validateEntity(_entity));
	return entityIdToComponentMap[_entity->id];
}

void EntityManager::addOnEntityCreatedListener(IOnEntityCreatedListener *_listener)
{
	assert(_listener);
	onEntityCreatedListeners.push_back(_listener);
}

void EntityManager::addOnEntityDestructionListener(IOnEntityDestructionListener *_listener)
{
	assert(_listener);
	onEntityDestructionListeners.push_back(_listener);
}

void EntityManager::addOnComponentAddedListener(IOnComponentAddedListener *_listener)
{
	assert(_listener);
	onComponentAddedListeners.push_back(_listener);
}

void EntityManager::addOnComponentRemovedListener(IOnComponentRemovedListener *_listener)
{
	assert(_listener);
	onComponentRemovedListeners.push_back(_listener);
}

void EntityManager::removeOnEntityCreatedListener(IOnEntityCreatedListener *_listener)
{
	assert(_listener);
	remove(onEntityCreatedListeners, _listener);
}

void EntityManager::removeOnEntityDestructionListener(IOnEntityDestructionListener *_listener)
{
	assert(_listener);
	remove(onEntityDestructionListeners, _listener);
}

void EntityManager::removeOnComponentAddedListener(IOnComponentAddedListener *_listener)
{
	assert(_listener);
	remove(onComponentAddedListeners, _listener);
}

void EntityManager::removeOnComponentRemovedListener(IOnComponentRemovedListener *_listener)
{
	assert(_listener);
	remove(onComponentRemovedListeners, _listener);
}

bool EntityManager::validateEntity(const Entity *_entity)
{
	assert(_entity);

	const Entity::Id &id = _entity->id;
	const Entity::Version &version = _entity->version;

	//check if given id is still a valid id and present in all maps
	return contains(entityIdVersionMap, id) &&
		entityIdVersionMap[id] == version &&
		contains(entityIdToComponentBitFieldMap, id) &&
		contains(entityIdToFamilyBitFieldMap, id) &&
		contains(entityIdToComponentMap, id) &&
		contains(entityIdToFamilyMap, id);
}
