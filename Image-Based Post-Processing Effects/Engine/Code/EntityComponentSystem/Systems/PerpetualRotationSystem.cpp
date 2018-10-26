#include "PerpetualRotationSystem.h"
#include <glm\gtx\quaternion.hpp>
#include ".\..\EntityManager.h"

PerpetualRotationSystem::PerpetualRotationSystem()
	:m_entityManager(EntityManager::getInstance())
{
	m_validBitMaps.push_back(Component<PerpetualRotationComponent>::getTypeId());
}

void PerpetualRotationSystem::init()
{
	m_entityManager.addOnComponentAddedListener(this);
	m_entityManager.addOnComponentRemovedListener(this);
	m_entityManager.addOnEntityDestructionListener(this);
}

void PerpetualRotationSystem::input(double _currentTime, double _timeDelta)
{
}

void PerpetualRotationSystem::update(double _currentTime, double _timeDelta)
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
		PerpetualRotationComponent *prc = m_entityManager.getComponent<PerpetualRotationComponent>(entity);
		TransformationComponent *tc = m_entityManager.getComponent<TransformationComponent>(entity);
		assert(prc && tc);
		tc->m_rotation *= glm::quat(prc->m_rotationIncrement * (float)_timeDelta);// glm::pow(prc->rotationIncrement, (float)_timeDelta);
	}
}

void PerpetualRotationSystem::render()
{
}

void PerpetualRotationSystem::onComponentAdded(const Entity *_entity, BaseComponent *_addedComponent)
{
	if (validate(m_entityManager.getComponentBitField(_entity)) && !ContainerUtility::contains(m_entitiesToAdd, _entity))
	{
		if (!ContainerUtility::contains(m_managedEntities, _entity) || ContainerUtility::contains(m_entitiesToRemove, _entity))
		{
			m_entitiesToAdd.push_back(_entity);
		}
	}
}

void PerpetualRotationSystem::onComponentRemoved(const Entity *_entity, BaseComponent *_removedComponent)
{
	if (!validate(m_entityManager.getComponentBitField(_entity)) && ContainerUtility::contains(m_managedEntities, _entity))
	{
		m_entitiesToRemove.push_back(_entity);
	}
}

void PerpetualRotationSystem::onDestruction(const Entity *_entity)
{
	if (ContainerUtility::contains(m_managedEntities, _entity))
	{
		m_entitiesToRemove.push_back(_entity);
	}
}

bool PerpetualRotationSystem::validate(std::uint64_t _bitMap)
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
