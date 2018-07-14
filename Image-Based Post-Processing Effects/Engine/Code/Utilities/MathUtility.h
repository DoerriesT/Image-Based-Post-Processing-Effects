#pragma once
#include <glm\gtc\quaternion.hpp>

namespace MathUtility
{
	// quaternion nlerp which chooses the shorter path
	glm::quat nlerp(const glm::quat &_x, const glm::quat &_y, float _a);
}