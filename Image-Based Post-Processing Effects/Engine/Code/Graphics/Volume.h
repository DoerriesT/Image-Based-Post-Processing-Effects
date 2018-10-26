#pragma once
#include <glm/vec3.hpp>

struct Volume
{
	glm::vec3 m_origin;
	glm::ivec3 m_dimensions;
	float m_spacing;
};