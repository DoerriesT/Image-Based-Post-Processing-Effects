#include "PerpetualRotationSystem.h"
#include <glm\gtx\quaternion.hpp>
#include ".\..\EntityManager.h"

PerpetualRotationSystem::PerpetualRotationSystem()
	:entityManager(EntityManager::getInstance())
{
	validBitMaps.push_back(Component<PerpetualRotationComponent>::getTypeId());
}

void PerpetualRotationSystem::init()
{
	entityManager.addOnComponentAddedListener(this);
	entityManager.addOnComponentRemovedListener(this);
	entityManager.addOnEntityDestructionListener(this);
}

void PerpetualRotationSystem::input(double _currentTime, double _timeDelta)
{
}

void PerpetualRotationSystem::update(double _currentTime, double _timeDelta)
{
	for (const Entity *entity : entitiesToRemove)
	{
		remove(managedEntities, entity);
	}
	entitiesToRemove.clear();

	for (const Entity *entity : entitiesToAdd)
	{
		managedEntities.push_back(entity);
	}
	entitiesToAdd.clear();

	for (const Entity *entity : managedEntities)
	{
		PerpetualRotationComponent *prc = entityManager.getComponent<PerpetualRotationComponent>(entity);
		TransformationComponent *tc = entityManager.getComponent<TransformationComponent>(entity);
		assert(prc && tc);
		tc->rotation *= glm::quat(prc->rotationIncrement * (float)_timeDelta);// glm::pow(prc->rotationIncrement, (float)_timeDelta);
	}
}

void PerpetualRotationSystem::render()
{
}

void PerpetualRotationSystem::onComponentAdded(const Entity *_entity, BaseComponent *_addedComponent)
{
	if (validate(entityManager.getComponentBitField(_entity)) && !contains(entitiesToAdd, _entity))
	{
		if (!contains(managedEntities, _entity) || contains(entitiesToRemove, _entity))
		{
			entitiesToAdd.push_back(_entity);
		}
	}
}

void PerpetualRotationSystem::onComponentRemoved(const Entity *_entity, BaseComponent *_removedComponent)
{
	if (!validate(entityManager.getComponentBitField(_entity)) && contains(managedEntities, _entity))
	{
		entitiesToRemove.push_back(_entity);
	}
}

void PerpetualRotationSystem::onDestruction(const Entity *_entity)
{
	if (contains(managedEntities, _entity))
	{
		entitiesToRemove.push_back(_entity);
	}
}

bool PerpetualRotationSystem::validate(std::uint64_t _bitMap)
{
	for (std::uint64_t configuration : validBitMaps)
	{
		if ((configuration & _bitMap) == configuration)
		{
			return true;
		}
	}
	return false;
}
