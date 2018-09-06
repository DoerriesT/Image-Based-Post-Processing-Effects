#pragma once
#include <glm/vec3.hpp>

struct Volume
{
	glm::vec3 origin;
	glm::ivec3 dimensions;
	float spacing;
};