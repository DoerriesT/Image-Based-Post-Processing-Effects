#include "MathUtility.h"

glm::quat MathUtility::nlerp(const glm::quat &_x, const glm::quat &_y, float _a)
{
	float cosom = _x.x * _y.x + _x.y * _y.y + _x.z * _y.z + _x.w * _y.w;
	float scale0 = 1.0f - _a;
	float scale1 = (cosom >= 0.0f) ? _a : -_a;
	glm::quat result;
	result = scale0 * _x + scale1 * _y;
	float s = (float)(1.0 / sqrt(result.x * result.x + result.y * result.y + result.z * result.z + result.w * result.w));
	result *= s;
	return result;
}