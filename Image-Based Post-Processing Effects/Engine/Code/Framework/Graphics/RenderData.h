#pragma once
#include <glm\mat4x4.hpp>
#include <glm\vec3.hpp>
#include <utility>

struct RenderData
{
	std::pair<unsigned int, unsigned int> resolution;
	glm::mat4 viewMatrix;
	glm::mat4 invViewMatrix;
	glm::mat4 projectionMatrix;
	glm::mat4 invProjectionMatrix;
	glm::mat4 viewProjectionMatrix;
	glm::mat4 invViewProjectionMatrix;
	glm::mat4 prevViewProjectionMatrix;
	glm::vec3 cameraPosition;
	glm::vec3 viewDirection;
	float time;
	bool shadows;
	float fov;
};