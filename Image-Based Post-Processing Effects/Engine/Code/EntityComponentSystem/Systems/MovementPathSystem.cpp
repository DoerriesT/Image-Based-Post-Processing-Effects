#include "MovementPathSystem.h"
#include <algorithm>
#include ".\..\..\Utilities\Utility.h"
#include ".\..\EntityManager.h"

MovementPathSystem::MovementPathSystem()
	:entityManager(EntityManager::getInstance())
{
	validBitMaps.push_back(Component<MovementPathComponent>::getTypeId());
}

void MovementPathSystem::init()
{
	entityManager.addOnComponentAddedListener(this);
	entityManager.addOnComponentRemovedListener(this);
	entityManager.addOnEntityDestructionListener(this);
}

void MovementPathSystem::input(double _currentTime, double _timeDelta)
{
}

void MovementPathSystem::update(double _currentTime, double _timeDelta)
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
		MovementPathComponent *mpc = entityManager.getComponent<MovementPathComponent>(entity);
		assert(mpc);

		if (_currentTime >= mpc->startTime)
		{
			TransformationComponent *tc = entityManager.getComponent<TransformationComponent>(entity);
			PathSegment *segment = &mpc->pathSegments[mpc->currentSegmentIndex];
			assert(tc && segment);
			
			float factor = static_cast<float>(segment->easingFunction(_currentTime - mpc->currentStartTime, segment->totalDuration));
			if (factor >= 1.0f)
			{
				// set position to segment end position
				tc->position = segment->endPosition;
				// update start time for next segment
				mpc->currentStartTime += segment->totalDuration;
				// increment segment index
				++mpc->currentSegmentIndex;

				// if we reached the last segment either wrap around or remove the component
				size_t totalSegments = mpc->pathSegments.size();
				if (mpc->currentSegmentIndex >= totalSegments)
				{
					if (mpc->repeat)
					{
						mpc->currentSegmentIndex %= totalSegments;
						segment->onCompleted();
					}
					else
					{
						std::function<void()> onCompleted = segment->onCompleted;
						entityManager.removeComponent<MovementPathComponent>(entity);
						onCompleted();
					}
				}
				else
				{
					segment->onCompleted();
				}
			}
			else
			{
				tc->position = interpolateHermiteCurve(factor, segment->startPosition, segment->endPosition, segment->startTangent, segment->endTangent);
			}
		}
	}
}

void MovementPathSystem::render()
{
}

void MovementPathSystem::onComponentAdded(const Entity *_entity, BaseComponent *_addedComponent)
{
	if (validate(entityManager.getComponentBitField(_entity)) && !contains(entitiesToAdd, _entity))
	{
		if (!contains(managedEntities, _entity) || contains(entitiesToRemove, _entity))
		{
			entitiesToAdd.push_back(_entity);
		}
	}
}

void MovementPathSystem::onComponentRemoved(const Entity *_entity, BaseComponent *_removedComponent)
{
	if (!validate(entityManager.getComponentBitField(_entity)) && contains(managedEntities, _entity))
	{
		entitiesToRemove.push_back(_entity);
	}
}

void MovementPathSystem::onDestruction(const Entity *_entity)
{
	if (contains(managedEntities, _entity))
	{
		entitiesToRemove.push_back(_entity);
	}
}

bool MovementPathSystem::validate(std::uint64_t _bitMap)
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
