#include "MovementPathSystem.h"
#include <algorithm>
#include ".\..\..\Utilities\Utility.h"
#include ".\..\EntityManager.h"
#include <glm\gtx\spline.hpp>

MovementPathSystem::MovementPathSystem()
	:m_entityManager(EntityManager::getInstance())
{
	m_validBitMaps.push_back(Component<MovementPathComponent>::getTypeId());
}

void MovementPathSystem::init()
{
	m_entityManager.addOnComponentAddedListener(this);
	m_entityManager.addOnComponentRemovedListener(this);
	m_entityManager.addOnEntityDestructionListener(this);
}

void MovementPathSystem::input(double _currentTime, double _timeDelta)
{
}

void MovementPathSystem::update(double _currentTime, double _timeDelta)
{
	for (const Entity *entity : m_entitiesToRemove)
	{
		ContainerUtility::remove(m_managedEntities, entity);
	}
	m_entitiesToRemove.clear();

	for (const Entity *entity : m_entitiesToAdd)
	{
		m_managedEntities.push_back(entity);
	}
	m_entitiesToAdd.clear();

	for (const Entity *entity : m_managedEntities)
	{
		MovementPathComponent *mpc = m_entityManager.getComponent<MovementPathComponent>(entity);
		assert(mpc);

		if (_currentTime >= mpc->m_startTime)
		{
			TransformationComponent *tc = m_entityManager.getComponent<TransformationComponent>(entity);
			PathSegment *segment = &mpc->m_pathSegments[mpc->m_currentSegmentIndex];
			assert(tc && segment);
			
			float factor = static_cast<float>(segment->m_easingFunction(_currentTime - mpc->m_currentStartTime, segment->m_totalDuration));
			if (factor >= 1.0f)
			{
				// set position to segment end position
				tc->m_position = segment->m_endPosition;
				// update start time for next segment
				mpc->m_currentStartTime += segment->m_totalDuration;
				// increment segment index
				++mpc->m_currentSegmentIndex;

				// if we reached the last segment either wrap around or remove the component
				size_t totalSegments = mpc->m_pathSegments.size();
				if (mpc->m_currentSegmentIndex >= totalSegments)
				{
					if (mpc->m_repeat)
					{
						mpc->m_currentSegmentIndex %= totalSegments;
						segment->m_onCompleted();
					}
					else
					{
						std::function<void()> onCompleted = segment->m_onCompleted;
						m_entityManager.removeComponent<MovementPathComponent>(entity);
						onCompleted();
					}
				}
				else
				{
					segment->m_onCompleted();
				}
			}
			else
			{
				tc->m_position = glm::hermite(segment->m_startPosition, segment->m_startTangent, segment->m_endPosition, segment->m_endTangent, factor);
			}
		}
	}
}

void MovementPathSystem::render()
{
}

void MovementPathSystem::onComponentAdded(const Entity *_entity, BaseComponent *_addedComponent)
{
	if (validate(m_entityManager.getComponentBitField(_entity)) && !ContainerUtility::contains(m_entitiesToAdd, _entity))
	{
		if (!ContainerUtility::contains(m_managedEntities, _entity) || ContainerUtility::contains(m_entitiesToRemove, _entity))
		{
			m_entitiesToAdd.push_back(_entity);
		}
	}
}

void MovementPathSystem::onComponentRemoved(const Entity *_entity, BaseComponent *_removedComponent)
{
	if (!validate(m_entityManager.getComponentBitField(_entity)) && ContainerUtility::contains(m_managedEntities, _entity))
	{
		m_entitiesToRemove.push_back(_entity);
	}
}

void MovementPathSystem::onDestruction(const Entity *_entity)
{
	if (ContainerUtility::contains(m_managedEntities, _entity))
	{
		m_entitiesToRemove.push_back(_entity);
	}
}

bool MovementPathSystem::validate(std::uint64_t _bitMap)
{
	for (std::uint64_t configuration : m_validBitMaps)
	{
		if ((configuration & _bitMap) == configuration)
		{
			return true;
		}
	}
	return false;
}
