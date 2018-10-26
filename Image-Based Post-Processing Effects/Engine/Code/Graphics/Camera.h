#pragma once
#include <glm\vec3.hpp>
#include <glm\mat4x4.hpp>
#include <glm\gtc\quaternion.hpp>
#include "Frustum.h"

class Camera
{
public:
	Camera() = default;
	explicit Camera(const glm::vec3 &_position, const glm::quat &_rotation);
	explicit Camera(const glm::vec3 &_position, const glm::vec3 &_pitchYawRoll);
	void setRotation(const glm::quat &_rotation);
	void setPosition(const glm::vec3 &_position);
	void rotate(const glm::vec3 &_pitchYawRollOffset);
	void translate(const glm::vec3 &_translationOffset);
	void lookAt(const glm::vec3 &_targetPosition);
	void setZoom(float _zoom);
	const glm::mat4 &getViewMatrix();
	const glm::vec3 &getPosition() const;
	const glm::quat &getRotation() const;
	const Frustum &getFrustum() const;
	glm::vec3 getForwardDirection();
	glm::vec3 getUpDirection();
	float getZoom() const;
	void reset();

private:
	glm::vec3 m_position;
	glm::quat m_rotation;
	Frustum m_frustum;

	glm::vec3 m_startPosition;
	glm::quat m_startRotation;

	glm::mat4 m_viewMatrix;
	float m_zoom = 1.0f;
	bool m_needToUpdateViewMatrix = true;

	void updateViewMatrix();
};