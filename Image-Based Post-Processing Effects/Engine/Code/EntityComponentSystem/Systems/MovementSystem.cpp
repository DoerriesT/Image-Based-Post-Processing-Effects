#include "MovementSystem.h"
#include <algorithm>
#include ".\..\..\Utilities\Utility.h"
#include ".\..\EntityManager.h"

MovementSystem::MovementSystem()
	:m_entityManager(EntityManager::getInstance())
{
	m_validBitMaps.push_back(Component<MovementComponent>::getTypeId());
}

void MovementSystem::init()
{
	m_entityManager.addOnComponentAddedListener(this);
	m_entityManager.addOnComponentRemovedListener(this);
	m_entityManager.addOnEntityDestructionListener(this);
}

void MovementSystem::input(double _currentTime, double _timeDelta)
{
}

void MovementSystem::update(double _currentTime, double _timeDelta)
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
		MovementComponent *mc = m_entityManager.getComponent<MovementComponent>(entity);
		assert(mc);

		if (_currentTime >= mc->m_startTime)
		{
			TransformationComponent *tc = m_entityManager.getComponent<TransformationComponent>(entity);
			assert(tc);
			float factor = static_cast<float>(mc->m_easingFunction(_currentTime - mc->m_startTime, mc->m_totalDuration));
			if (factor >= 1.0f)
			{
				tc->m_position = mc->m_endPosition;
				std::function<void()> onCompleted = mc->m_onCompleted;
				m_entityManager.removeComponent<MovementComponent>(entity);
				onCompleted();
			}
			else
			{
				//float elevation = factor > 0.5f ? (1.0f - factor) * mc.getElevation() : factor * mc.getElevation();
				tc->m_position = mc->m_startPosition + factor * mc->m_path;
			}
		}
	}
}

void MovementSystem::render()
{
}

void MovementSystem::onComponentAdded(const Entity *_entity, BaseComponent *_addedComponent)
{
	if (validate(m_entityManager.getComponentBitField(_entity)) && !ContainerUtility::contains(m_entitiesToAdd, _entity))
	{
		if (!ContainerUtility::contains(m_managedEntities, _entity) || ContainerUtility::contains(m_entitiesToRemove, _entity))
		{
			m_entitiesToAdd.push_back(_entity);
		}
	}
}

void MovementSystem::onComponentRemoved(const Entity *_entity, BaseComponent *_removedComponent)
{
	if (!validate(m_entityManager.getComponentBitField(_entity)) && ContainerUtility::contains(m_managedEntities, _entity))
	{
		m_entitiesToRemove.push_back(_entity);
	}
}

void MovementSystem::onDestruction(const Entity *_entity)
{
	if (ContainerUtility::contains(m_managedEntities, _entity))
	{
		m_entitiesToRemove.push_back(_entity);
	}
}

bool MovementSystem::validate(std::uint64_t _bitMap)
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
