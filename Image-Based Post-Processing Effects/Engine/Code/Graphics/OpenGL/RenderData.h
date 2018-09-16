#pragma once
#include <glm\mat4x4.hpp>
#include <glm\vec3.hpp>
#include <utility>
#include "Graphics\Frustum.h"

struct RenderData
{
	Frustum frustum;
	std::pair<unsigned int, unsigned int> resolution;
	glm::mat4 viewMatrix;
	glm::mat4 invViewMatrix;
	glm::mat4 projectionMatrix;
	glm::mat4 invProjectionMatrix;
	glm::mat4 viewProjectionMatrix;
	glm::mat4 invViewProjectionMatrix;
	glm::mat4 prevViewProjectionMatrix;
	glm::mat4 invJitter;
	glm::mat4 prevInvJitter;
	glm::vec3 cameraPosition;
	glm::vec3 viewDirection;
	float time;
	bool shadows;
	float fov;
	unsigned int frame;
	bool bake;
};