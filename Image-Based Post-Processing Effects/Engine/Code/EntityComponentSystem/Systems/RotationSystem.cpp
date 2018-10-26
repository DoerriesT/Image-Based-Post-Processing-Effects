#include "RotationSystem.h"
#include <algorithm>
#include "Utilities\ContainerUtility.h"
#include "Utilities\MathUtility.h"
#include ".\..\EntityManager.h"

RotationSystem::RotationSystem()
	:m_entityManager(EntityManager::getInstance())
{
	m_validBitMaps.push_back(Component<RotationComponent>::getTypeId());
}

void RotationSystem::init()
{
	m_entityManager.addOnComponentAddedListener(this);
	m_entityManager.addOnComponentRemovedListener(this);
	m_entityManager.addOnEntityDestructionListener(this);
}

void RotationSystem::input(double _currentTime, double _timeDelta)
{
}

void RotationSystem::update(double _currentTime, double _timeDelta)
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
		RotationComponent *rc = m_entityManager.getComponent<RotationComponent>(entity);
		
		assert(rc);

		if (_currentTime >= rc->m_startTime)
		{
			TransformationComponent *tc = m_entityManager.getComponent<TransformationComponent>(entity);
			assert(tc);
			float factor = static_cast<float>(rc->m_easingFunction(_currentTime - rc->m_startTime, rc->m_totalDuration));
			if (factor >= 1.0f)
			{
				tc->m_rotation = rc->m_endRotation;
				std::function<void()> onCompleted = rc->m_onCompleted;
				m_entityManager.removeComponent<RotationComponent>(entity);
				onCompleted();
			}
			else
			{
				//tc->rotation = glm::normalize(glm::lerp(rc->startRotation, rc->endRotation, factor));
				//tc->rotation = glm::mix(rc->startRotation, rc->endRotation, factor);
				tc->m_rotation = MathUtility::nlerp(rc->m_startRotation, rc->m_endRotation, factor);
			}
		}
	}
}

void RotationSystem::render()
{
}

void RotationSystem::onComponentAdded(const Entity *_entity, BaseComponent *_addedComponent)
{
	if (validate(m_entityManager.getComponentBitField(_entity)) && !ContainerUtility::contains(m_entitiesToAdd, _entity))
	{
		if (!ContainerUtility::contains(m_managedEntities, _entity) || ContainerUtility::contains(m_entitiesToRemove, _entity))
		{
			m_entitiesToAdd.push_back(_entity);
		}
	}
}

void RotationSystem::onComponentRemoved(const Entity *_entity, BaseComponent *_removedComponent)
{
	if (!validate(m_entityManager.getComponentBitField(_entity)) && ContainerUtility::contains(m_managedEntities, _entity))
	{
		m_entitiesToRemove.push_back(_entity);
	}
}

void RotationSystem::onDestruction(const Entity *_entity)
{
	if (ContainerUtility::contains(m_managedEntities, _entity))
	{
		m_entitiesToRemove.push_back(_entity);
	}
}

bool RotationSystem::validate(std::uint64_t _bitMap)
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
