#include <algorithm>
#include "EntityManager.h"

Entity::Entity(const Id &_id, const Id &_version)
	: m_id(_id), m_version(_version)
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
	if (m_freeIds.empty())
	{
		id = m_nextFreeId++;
	}
	else
	{
		id = m_freeIds.back();
		m_freeIds.pop_back();
	}
	Entity *entity = new Entity(id, m_entityIdVersionMap[id]);
	m_entityIdToComponentBitFieldMap[id] = 0;
	m_entityIdToComponentMap[id].clear();
	m_entityIdToFamilyBitFieldMap[id] = 0;
	m_entityIdToFamilyMap[id].clear();

	//invoke listeners
	for (IOnEntityCreatedListener *listener : m_onEntityCreatedListeners)
	{
		listener->onEntityCreated(entity);
	}

	return entity;
}


void EntityManager::removeComponentFamily(const Entity *_entity, std::uint64_t _familyId, bool _notify)
{
	assert(validateEntity(_entity));

	const Entity::Id &id = _entity->m_id;

	// only do something if a component of this family is actually attached to the entity
	if (m_entityIdToFamilyBitFieldMap[id] & _familyId)
	{
		// fetch the component pointer
		BaseComponent *component = m_entityIdToFamilyMap[id][_familyId];
		assert(component);

		std::uint64_t typeId = component->getTypeIdOfDerived();

		// remove type and family bits from their map
		m_entityIdToComponentBitFieldMap[id] ^= typeId;
		m_entityIdToFamilyBitFieldMap[id] ^= _familyId;

		// invoke listeners
		if (_notify)
		{
			for (IOnComponentRemovedListener *listener : m_onComponentRemovedListeners)
			{
				listener->onComponentRemoved(_entity, component);
			}
		}

		// delete pointer and remove component from maps
		delete component;
		ContainerUtility::remove(m_entityIdToComponentMap[id], typeId);
		ContainerUtility::remove(m_entityIdToFamilyMap[id], _familyId);
	}
}


void EntityManager::destroyEntity(const Entity *_entity)
{
	assert(validateEntity(_entity));

	const Entity::Id &id = _entity->m_id;

	//invoke listeners
	for (IOnEntityDestructionListener *listener : m_onEntityDestructionListeners)
	{
		listener->onDestruction(_entity);
	}

	for (std::uint64_t i = 0, familyId = 1; i <= MAX_FAMILY_ID; ++i, familyId = 1ui64 << i)
	{
		removeComponentFamily(_entity, familyId, false);
	}

	// bitmaps should already be zero
	assert(m_entityIdToComponentBitFieldMap[id] == 0 && m_entityIdToFamilyBitFieldMap[id] == 0);
	// reset bitmaps
	// entityIdToComponentBitFieldMap[id] = 0;
	// entityIdToFamilyBitFieldMap[id] = 0;
	// increase version
	++m_entityIdVersionMap[id];
	m_freeIds.push_back(id);

	delete _entity;
}

std::uint64_t EntityManager::getComponentBitField(const Entity *_entity)
{
	assert(validateEntity(_entity));
	return m_entityIdToComponentBitFieldMap[_entity->m_id];
}

std::unordered_map<std::uint64_t, BaseComponent *> EntityManager::getComponentMap(const Entity *_entity)
{
	assert(validateEntity(_entity));
	return m_entityIdToComponentMap[_entity->m_id];
}

void EntityManager::addOnEntityCreatedListener(IOnEntityCreatedListener *_listener)
{
	assert(_listener);
	m_onEntityCreatedListeners.push_back(_listener);
}

void EntityManager::addOnEntityDestructionListener(IOnEntityDestructionListener *_listener)
{
	assert(_listener);
	m_onEntityDestructionListeners.push_back(_listener);
}

void EntityManager::addOnComponentAddedListener(IOnComponentAddedListener *_listener)
{
	assert(_listener);
	m_onComponentAddedListeners.push_back(_listener);
}

void EntityManager::addOnComponentRemovedListener(IOnComponentRemovedListener *_listener)
{
	assert(_listener);
	m_onComponentRemovedListeners.push_back(_listener);
}

void EntityManager::removeOnEntityCreatedListener(IOnEntityCreatedListener *_listener)
{
	assert(_listener);
	ContainerUtility::remove(m_onEntityCreatedListeners, _listener);
}

void EntityManager::removeOnEntityDestructionListener(IOnEntityDestructionListener *_listener)
{
	assert(_listener);
	ContainerUtility::remove(m_onEntityDestructionListeners, _listener);
}

void EntityManager::removeOnComponentAddedListener(IOnComponentAddedListener *_listener)
{
	assert(_listener);
	ContainerUtility::remove(m_onComponentAddedListeners, _listener);
}

void EntityManager::removeOnComponentRemovedListener(IOnComponentRemovedListener *_listener)
{
	assert(_listener);
	ContainerUtility::remove(m_onComponentRemovedListeners, _listener);
}

bool EntityManager::validateEntity(const Entity *_entity)
{
	assert(_entity);

	const Entity::Id &id = _entity->m_id;
	const Entity::Version &version = _entity->m_version;

	//check if given id is still a valid id and present in all maps
	return ContainerUtility::contains(m_entityIdVersionMap, id) &&
		m_entityIdVersionMap[id] == version &&
		ContainerUtility::contains(m_entityIdToComponentBitFieldMap, id) &&
		ContainerUtility::contains(m_entityIdToFamilyBitFieldMap, id) &&
		ContainerUtility::contains(m_entityIdToComponentMap, id) &&
		ContainerUtility::contains(m_entityIdToFamilyMap, id);
}
