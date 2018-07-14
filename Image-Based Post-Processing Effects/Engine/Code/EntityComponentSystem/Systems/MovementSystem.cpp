#include "MovementSystem.h"
#include <algorithm>
#include ".\..\..\Utilities\Utility.h"
#include ".\..\EntityManager.h"

MovementSystem::MovementSystem()
	:entityManager(EntityManager::getInstance())
{
	validBitMaps.push_back(Component<MovementComponent>::getTypeId());
}

void MovementSystem::init()
{
	entityManager.addOnComponentAddedListener(this);
	entityManager.addOnComponentRemovedListener(this);
	entityManager.addOnEntityDestructionListener(this);
}

void MovementSystem::input(double _currentTime, double _timeDelta)
{
}

void MovementSystem::update(double _currentTime, double _timeDelta)
{
	for (const Entity *entity : entitiesToRemove)
	{
		ContainerUtility::remove(managedEntities, entity);
	}
	entitiesToRemove.clear();

	for (const Entity *entity : entitiesToAdd)
	{
		managedEntities.push_back(entity);
	}
	entitiesToAdd.clear();

	for (const Entity *entity : managedEntities)
	{
		MovementComponent *mc = entityManager.getComponent<MovementComponent>(entity);
		assert(mc);

		if (_currentTime >= mc->startTime)
		{
			TransformationComponent *tc = entityManager.getComponent<TransformationComponent>(entity);
			assert(tc);
			float factor = static_cast<float>(mc->easingFunction(_currentTime - mc->startTime, mc->totalDuration));
			if (factor >= 1.0f)
			{
				tc->position = mc->endPosition;
				std::function<void()> onCompleted = mc->onCompleted;
				entityManager.removeComponent<MovementComponent>(entity);
				onCompleted();
			}
			else
			{
				//float elevation = factor > 0.5f ? (1.0f - factor) * mc.getElevation() : factor * mc.getElevation();
				tc->position = mc->startPosition + factor * mc->path;
			}
		}
	}
}

void MovementSystem::render()
{
}

void MovementSystem::onComponentAdded(const Entity *_entity, BaseComponent *_addedComponent)
{
	if (validate(entityManager.getComponentBitField(_entity)) && !ContainerUtility::contains(entitiesToAdd, _entity))
	{
		if (!ContainerUtility::contains(managedEntities, _entity) || ContainerUtility::contains(entitiesToRemove, _entity))
		{
			entitiesToAdd.push_back(_entity);
		}
	}
}

void MovementSystem::onComponentRemoved(const Entity *_entity, BaseComponent *_removedComponent)
{
	if (!validate(entityManager.getComponentBitField(_entity)) && ContainerUtility::contains(managedEntities, _entity))
	{
		entitiesToRemove.push_back(_entity);
	}
}

void MovementSystem::onDestruction(const Entity *_entity)
{
	if (ContainerUtility::contains(managedEntities, _entity))
	{
		entitiesToRemove.push_back(_entity);
	}
}

bool MovementSystem::validate(std::uint64_t _bitMap)
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
