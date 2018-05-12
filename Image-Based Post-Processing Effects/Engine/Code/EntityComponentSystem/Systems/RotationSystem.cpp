#include "RotationSystem.h"
#include <algorithm>
#include ".\..\..\Utilities\Utility.h"
#include ".\..\EntityManager.h"

RotationSystem::RotationSystem()
	:entityManager(EntityManager::getInstance())
{
	validBitMaps.push_back(Component<RotationComponent>::getTypeId());
}

void RotationSystem::init()
{
	entityManager.addOnComponentAddedListener(this);
	entityManager.addOnComponentRemovedListener(this);
	entityManager.addOnEntityDestructionListener(this);
}

void RotationSystem::input(double _currentTime, double _timeDelta)
{
}

void RotationSystem::update(double _currentTime, double _timeDelta)
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
		RotationComponent *rc = entityManager.getComponent<RotationComponent>(entity);
		
		assert(rc);

		if (_currentTime >= rc->startTime)
		{
			TransformationComponent *tc = entityManager.getComponent<TransformationComponent>(entity);
			assert(tc);
			float factor = static_cast<float>(rc->easingFunction(_currentTime - rc->startTime, rc->totalDuration));
			if (factor >= 1.0f)
			{
				tc->rotation = rc->endRotation;
				std::function<void()> onCompleted = rc->onCompleted;
				entityManager.removeComponent<RotationComponent>(entity);
				onCompleted();
			}
			else
			{
				//tc->rotation = glm::normalize(glm::lerp(rc->startRotation, rc->endRotation, factor));
				//tc->rotation = glm::mix(rc->startRotation, rc->endRotation, factor);
				tc->rotation = nlerp(rc->startRotation, rc->endRotation, factor);
			}
		}
	}
}

void RotationSystem::render()
{
}

void RotationSystem::onComponentAdded(const Entity *_entity, BaseComponent *_addedComponent)
{
	if (validate(entityManager.getComponentBitField(_entity)) && !contains(entitiesToAdd, _entity))
	{
		if (!contains(managedEntities, _entity) || contains(entitiesToRemove, _entity))
		{
			entitiesToAdd.push_back(_entity);
		}
	}
}

void RotationSystem::onComponentRemoved(const Entity *_entity, BaseComponent *_removedComponent)
{
	if (!validate(entityManager.getComponentBitField(_entity)) && contains(managedEntities, _entity))
	{
		entitiesToRemove.push_back(_entity);
	}
}

void RotationSystem::onDestruction(const Entity *_entity)
{
	if (contains(managedEntities, _entity))
	{
		entitiesToRemove.push_back(_entity);
	}
}

bool RotationSystem::validate(std::uint64_t _bitMap)
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
