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
	:window(std::move(_window)),
	entityManager(EntityManager::getInstance())
{
	validBitMaps.push_back(Component<GrabbedComponent>::getTypeId() | Component<TransformationComponent>::getTypeId());
}

void GrabbingSystem::init()
{
	entityManager.addOnComponentAddedListener(this);
	entityManager.addOnComponentRemovedListener(this);
	entityManager.addOnEntityDestructionListener(this);
}

void GrabbingSystem::input(double _currentTime, double _timeDelta)
{
}

void GrabbingSystem::update(double _currentTime, double _timeDelta)
{
	std::shared_ptr<Level> level = SystemManager::getInstance().getLevel();
	if (!level || level->cameras.empty())
	{
		// we cant do any grabbing if there is no active camera
		return;
	}

	std::shared_ptr<Camera> camera = level->cameras[level->activeCameraIndex];

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

	glm::vec3 mouseDir = getMouseDirection(window, &*camera);

	for (const Entity *entity : managedEntities)
	{
		GrabbedComponent *gc = entityManager.getComponent<GrabbedComponent>(entity);
		TransformationComponent *tc = entityManager.getComponent<TransformationComponent>(entity);
		assert(gc && tc);

		float distance;
		if (glm::intersectRayPlane(camera->getPosition(), mouseDir, gc->planeOrigin, gc->planeNormal, distance))
		{
			tc->position = camera->getPosition() + mouseDir * distance;
		}
	}
}

void GrabbingSystem::render()
{
}

void GrabbingSystem::onComponentAdded(const Entity *_entity, BaseComponent *_addedComponent)
{
	if (validate(entityManager.getComponentBitField(_entity)) && !ContainerUtility::contains(entitiesToAdd, _entity))
	{
		if (!ContainerUtility::contains(managedEntities, _entity) || ContainerUtility::contains(entitiesToRemove, _entity))
		{
			entitiesToAdd.push_back(_entity);
		}
	}
}

void GrabbingSystem::onComponentRemoved(const Entity *_entity, BaseComponent *_removedComponent)
{
	if (!validate(entityManager.getComponentBitField(_entity)) && ContainerUtility::contains(managedEntities, _entity))
	{
		entitiesToRemove.push_back(_entity);
	}
}

void GrabbingSystem::onDestruction(const Entity *_entity)
{
	if (ContainerUtility::contains(managedEntities, _entity))
	{
		entitiesToRemove.push_back(_entity);
	}
}

bool GrabbingSystem::validate(std::uint64_t _bitMap)
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
