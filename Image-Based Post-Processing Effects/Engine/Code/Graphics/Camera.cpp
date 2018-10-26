#include <glm\gtc\matrix_transform.hpp>
#include "Camera.h"
#include "EntityComponentSystem\SystemManager.h"
#include "EntityComponentSystem\Systems\RenderSystem.h"
#include "Window\Window.h"

Camera::Camera(const glm::vec3 &_position, const glm::quat &_rotation)
	: m_position(_position), m_rotation(_rotation), m_startPosition(_position), m_startRotation(_rotation)
{
}

Camera::Camera(const glm::vec3 & _position, const glm::vec3 &_pitchYawRoll)
	: m_position(_position), m_rotation(_pitchYawRoll), m_startPosition(_position), m_startRotation(_pitchYawRoll)
{
}

void Camera::setRotation(const glm::quat &_rotation)
{
	m_rotation = _rotation;
	m_needToUpdateViewMatrix = true;
}

void Camera::setPosition(const glm::vec3 &_position)
{
	m_position = _position;
	m_needToUpdateViewMatrix = true;
}

void Camera::rotate(const glm::vec3 &_pitchYawRoll)
{
	glm::quat tmp = glm::quat(glm::vec3(_pitchYawRoll.x, 0.0, 0.0));
	glm::quat tmp1 = glm::quat(glm::angleAxis(_pitchYawRoll.y, glm::vec3(0.0, 1.0, 0.0)));
	m_rotation = glm::normalize(tmp * m_rotation * tmp1);

	m_needToUpdateViewMatrix = true;
}

void Camera::translate(const glm::vec3 &_translation)
{
	glm::vec3 forward(m_viewMatrix[0][2], m_viewMatrix[1][2], m_viewMatrix[2][2]);
	glm::vec3 strafe(m_viewMatrix[0][0], m_viewMatrix[1][0], m_viewMatrix[2][0]);

	static const float speed = 1.12f;

	m_position += (_translation.z * forward + _translation.x * strafe) * speed;
	m_needToUpdateViewMatrix = true;
}

void Camera::lookAt(const glm::vec3 &_targetPosition)
{
	static const glm::vec3 UP(0.0, 1.0f, 0.0);

	m_viewMatrix = glm::lookAt(m_position, _targetPosition, UP);
	glm::quat rot = glm::quat_cast(m_viewMatrix);
	m_needToUpdateViewMatrix = false;
}

void Camera::setZoom(float _zoom)
{
	assert(m_zoom > 0.0f);
	m_zoom = _zoom;
	SystemManager::getInstance().getSystem<RenderSystem>()->getWindow()->setFieldOfView(Window::DEFAULT_FOV / m_zoom);
}

const glm::mat4 &Camera::getViewMatrix()
{
	if (m_needToUpdateViewMatrix)
	{
		updateViewMatrix();
		m_needToUpdateViewMatrix = false;
	}
	return m_viewMatrix;
}

const glm::vec3 &Camera::getPosition() const
{
	return m_position;
}

const glm::quat &Camera::getRotation() const
{
	return m_rotation;
}

const Frustum &Camera::getFrustum() const
{
	return m_frustum;
}

glm::vec3 Camera::getForwardDirection()
{
	return -glm::transpose(getViewMatrix())[2];
}

glm::vec3 Camera::getUpDirection()
{
	return glm::transpose(getViewMatrix())[1];
}

float Camera::getZoom() const
{
	return m_zoom;
}

void Camera::reset()
{
	m_position = m_startPosition;
	m_rotation = m_startRotation;
	m_needToUpdateViewMatrix = true;
}

void Camera::updateViewMatrix()
{
	glm::mat4 translate;
	m_viewMatrix = glm::mat4_cast(m_rotation) * glm::translate(translate, -m_position);

	// TODO: update this also when projection changes
	m_frustum.update(SystemManager::getInstance().getSystem<RenderSystem>()->getWindow()->getProjectionMatrix() * m_viewMatrix);
}