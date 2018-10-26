#pragma once
#include <glm\mat4x4.hpp>
#include <glm\vec3.hpp>
#include <utility>
#include "Graphics\Frustum.h"

struct RenderData
{
	Frustum m_frustum;
	std::pair<unsigned int, unsigned int> m_resolution;
	glm::mat4 m_viewMatrix;
	glm::mat4 m_invViewMatrix;
	glm::mat4 m_projectionMatrix;
	glm::mat4 m_invProjectionMatrix;
	glm::mat4 m_viewProjectionMatrix;
	glm::mat4 m_invViewProjectionMatrix;
	glm::mat4 m_prevViewProjectionMatrix;
	glm::mat4 m_invJitter;
	glm::mat4 m_prevInvJitter;
	glm::vec3 m_cameraPosition;
	glm::vec3 m_viewDirection;
	float m_time;
	bool m_shadows;
	float m_fov;
	float m_nearPlane;
	float m_farPlane;
	unsigned int m_frame;
	bool m_bake;
};