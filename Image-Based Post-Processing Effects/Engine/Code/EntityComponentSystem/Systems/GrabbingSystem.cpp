#include <glm\gtx\intersect.hpp>
#include "GrabbingSystem.h"
#include "Input\UserInput.h"
#include "Utilities\Intersection.h"
#include "Utilities\Utility.h"
#include ".\..\SystemManager.h"
#include ".\..\EntityManager.h"
#include "Window\Window.h"
#include "Level.h"

GrabbingSystem::GrabbingSystem(std::shared_ptr<Window>_window)
	:m_window(std::move(_window)),
	m_entityManager(EntityManager::getInstance())
{
	m_validBitMaps.push_back(Component<GrabbedComponent>::getTypeId() | Component<TransformationComponent>::getTypeId());
}

void GrabbingSystem::init()
{
	m_entityManager.addOnComponentAddedListener(this);
	m_entityManager.addOnComponentRemovedListener(this);
	m_entityManager.addOnEntityDestructionListener(this);
}

void GrabbingSystem::input(double _currentTime, double _timeDelta)
{
}

void GrabbingSystem::update(double _currentTime, double _timeDelta)
{
	std::shared_ptr<Level> level = SystemManager::getInstance().getLevel();
	if (!level || level->m_cameras.empty())
	{
		// we cant do any grabbing if there is no active camera
		return;
	}

	std::shared_ptr<Camera> camera = level->m_cameras[level->m_activeCameraIndex];

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

	glm::vec3 mouseDir = getMouseDirection(m_window, &*camera);

	for (const Entity *entity : m_managedEntities)
	{
		GrabbedComponent *gc = m_entityManager.getComponent<GrabbedComponent>(entity);
		TransformationComponent *tc = m_entityManager.getComponent<TransformationComponent>(entity);
		assert(gc && tc);

		float distance;
		if (glm::intersectRayPlane(camera->getPosition(), mouseDir, gc->m_planeOrigin, gc->m_planeNormal, distance))
		{
			tc->m_position = camera->getPosition() + mouseDir * distance;
		}
	}
}

void GrabbingSystem::render()
{
}

void GrabbingSystem::onComponentAdded(const Entity *_entity, BaseComponent *_addedComponent)
{
	if (validate(m_entityManager.getComponentBitField(_entity)) && !ContainerUtility::contains(m_entitiesToAdd, _entity))
	{
		if (!ContainerUtility::contains(m_managedEntities, _entity) || ContainerUtility::contains(m_entitiesToRemove, _entity))
		{
			m_entitiesToAdd.push_back(_entity);
		}
	}
}

void GrabbingSystem::onComponentRemoved(const Entity *_entity, BaseComponent *_removedComponent)
{
	if (!validate(m_entityManager.getComponentBitField(_entity)) && ContainerUtility::contains(m_managedEntities, _entity))
	{
		m_entitiesToRemove.push_back(_entity);
	}
}

void GrabbingSystem::onDestruction(const Entity *_entity)
{
	if (ContainerUtility::contains(m_managedEntities, _entity))
	{
		m_entitiesToRemove.push_back(_entity);
	}
}

bool GrabbingSystem::validate(std::uint64_t _bitMap)
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
