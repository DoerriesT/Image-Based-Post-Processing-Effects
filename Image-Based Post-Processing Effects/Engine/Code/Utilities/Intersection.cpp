#include <limits>
#include "Intersection.h"
#include "Input\UserInput.h"
#include "EntityComponentSystem\EntityManager.h"
#include "Window\Window.h"
#include "Graphics\Camera.h"

bool intersectRayAabb(const glm::vec3 &_origin, const glm::vec3 &_dir, const glm::vec3 &_maxCorner, glm::vec2 &_result, const glm::vec3 &_position)
{
	glm::vec3 max = _maxCorner;
	glm::vec3 min = -max;
	min += _position;
	max += _position;
	float invDirX = 1.0f / _dir.x;
	float invDirY = 1.0f / _dir.y;
	float invDirZ = 1.0f / _dir.z;
	float tNear, tFar, tymin, tymax, tzmin, tzmax;
	if (invDirX >= 0.0f)
	{
		tNear = (min.x - _origin.x) * invDirX;
		tFar = (max.x - _origin.x) * invDirX;
	}
	else
	{
		tNear = (max.x - _origin.x) * invDirX;
		tFar = (min.x - _origin.x) * invDirX;
	}
	if (invDirY >= 0.0f)
	{
		tymin = (min.y - _origin.y) * invDirY;
		tymax = (max.y - _origin.y) * invDirY;
	}
	else
	{
		tymin = (max.y - _origin.y) * invDirY;
		tymax = (min.y - _origin.y) * invDirY;
	}
	if (tNear > tymax || tymin > tFar)
	{
		return false;
	}
	if (invDirZ >= 0.0f)
	{
		tzmin = (min.z - _origin.z) * invDirZ;
		tzmax = (max.z - _origin.z) * invDirZ;
	}
	else
	{
		tzmin = (max.z - _origin.z) * invDirZ;
		tzmax = (min.z - _origin.z) * invDirZ;
	}
	if (tNear > tzmax || tzmin > tFar)
	{
		return false;
	}
	tNear = (tymin > tNear || std::numeric_limits<float>::infinity() == tNear) ? tymin : tNear;
	tFar = (tymax < tFar || std::numeric_limits<float>::infinity() == tFar) ? tymax : tFar;
	tNear = tzmin > tNear ? tzmin : tNear;
	tFar = tzmax < tFar ? tzmax : tFar;
	if (tNear < tFar && tFar >= 0.0f)
	{
		_result.x = tNear;
		_result.y = tFar;
		return true;
	}
	return false;
}

bool intersectRayObb(const glm::vec3 &_origin, const glm::vec3 &_dir, const glm::vec3 &_maxCorner, const glm::vec3 &_position, const glm::quat &_rotation, glm::vec2 &_result)
{
	glm::quat invRotation = glm::inverse(_rotation);
	return intersectRayAabb(invRotation * (_origin - _position),  invRotation * _dir, _maxCorner, _result);
}

const Entity *getSelectedEntity(const std::vector<const Entity*> &_entities, const glm::vec3 &_cameraPosition, const glm::vec3 &_mouseDirection, bool _oriented)
{
	const Entity *closestEntity = nullptr;
	float closestDistance = std::numeric_limits<float>::max();
	glm::vec2 nearFar;
	EntityManager &entityManager = EntityManager::getInstance();

	if (_oriented)
	{
		for (const Entity *entity : _entities)
		{
			BoundingBoxComponent *boundingBoxComponent = entityManager.getComponent<BoundingBoxComponent>(entity);
			TransformationComponent *transformationComponent = entityManager.getComponent<TransformationComponent>(entity);
			if (!boundingBoxComponent)
				continue;
			assert(boundingBoxComponent && transformationComponent);

			if (intersectRayObb(_cameraPosition, _mouseDirection, boundingBoxComponent->maxCorner, transformationComponent->position, boundingBoxComponent->rotation*transformationComponent->rotation, nearFar) && nearFar.x < closestDistance)
			{
				closestDistance = nearFar.x;
				closestEntity = entity;
			}
		}
	}
	else
	{
		for (const Entity *entity : _entities)
		{
			BoundingBoxComponent *boundingBoxComponent = entityManager.getComponent<BoundingBoxComponent>(entity);
			TransformationComponent *transformationComponent = entityManager.getComponent<TransformationComponent>(entity);
			assert(boundingBoxComponent && transformationComponent);

			if (intersectRayAabb(_cameraPosition, _mouseDirection, boundingBoxComponent->maxCorner, nearFar, transformationComponent->position) && nearFar.x < closestDistance)
			{
				closestDistance = nearFar.x;
				closestEntity = entity;
			}
		}
	}

	return closestEntity;
}

glm::vec3 getMouseDirection(const std::shared_ptr<Window> &_window, Camera *_camera)
{
	glm::vec2 mousePos = UserInput::getInstance().getCurrentMousePos();
	double x = (2.0 * mousePos.x) / _window->getWidth() - 1.0;
	double y = 1.0 - (2.0 * mousePos.y) / _window->getHeight();
	double z = -1.0;

	glm::mat4 invProjectionMatrix = glm::inverse(_window->getProjectionMatrix());

	glm::vec4 mouseDir(x, y, z, 1.0);
	mouseDir = invProjectionMatrix * mouseDir;
	mouseDir.z = -1.0f;
	mouseDir.w = 0.0f;

	mouseDir = glm::inverse(_camera->getViewMatrix()) * mouseDir;

	return glm::normalize(mouseDir);
}
