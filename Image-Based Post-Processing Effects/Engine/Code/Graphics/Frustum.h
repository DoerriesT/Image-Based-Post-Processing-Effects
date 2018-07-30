#pragma once
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

class Frustum
{
public:
	explicit Frustum(const glm::mat4 &_viewProjection = glm::mat4());
	void update(const glm::mat4 &_viewProjection);
	// returns true if sphere intersects or is contained by frustum
	bool testSphere(const glm::vec4 &_sphere) const;

private:
	glm::vec4 planes[6];
};