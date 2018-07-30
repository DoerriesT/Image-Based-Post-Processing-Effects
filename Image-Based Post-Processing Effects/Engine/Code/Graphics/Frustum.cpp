#include "Frustum.h"
#include <glm/geometric.hpp>

Frustum::Frustum(const glm::mat4 &_viewProjection)
{
	update(_viewProjection);
}

void Frustum::update(const glm::mat4 &_viewProjection)
{
	// Left clipping plane
	planes[0].x = _viewProjection[0][3] + _viewProjection[0][0];
	planes[0].y = _viewProjection[1][3] + _viewProjection[1][0];
	planes[0].z = _viewProjection[2][3] + _viewProjection[2][0];
	planes[0].w = _viewProjection[3][3] + _viewProjection[3][0];
	// Right clipping plane
	planes[1].x = _viewProjection[0][3] - _viewProjection[0][0];
	planes[1].y = _viewProjection[1][3] - _viewProjection[1][0];
	planes[1].z = _viewProjection[2][3] - _viewProjection[2][0];
	planes[1].w = _viewProjection[3][3] - _viewProjection[3][0];
	// Top clipping plane
	planes[2].x = _viewProjection[0][3] - _viewProjection[0][1];
	planes[2].y = _viewProjection[1][3] - _viewProjection[1][1];
	planes[2].z = _viewProjection[2][3] - _viewProjection[2][1];
	planes[2].w = _viewProjection[3][3] - _viewProjection[3][1];
	// Bottom clipping plane
	planes[3].x = _viewProjection[0][3] + _viewProjection[0][1];
	planes[3].y = _viewProjection[1][3] + _viewProjection[1][1];
	planes[3].z = _viewProjection[2][3] + _viewProjection[2][1];
	planes[3].w = _viewProjection[3][3] + _viewProjection[3][1];
	// Near clipping plane
	planes[4].x = _viewProjection[0][3] + _viewProjection[0][2];
	planes[4].y = _viewProjection[1][3] + _viewProjection[1][2];
	planes[4].z = _viewProjection[2][3] + _viewProjection[2][2];
	planes[4].w = _viewProjection[3][3] + _viewProjection[3][2];
	// Far clipping plane
	planes[5].x = _viewProjection[0][3] - _viewProjection[0][2];
	planes[5].y = _viewProjection[1][3] - _viewProjection[1][2];
	planes[5].z = _viewProjection[2][3] - _viewProjection[2][2];
	planes[5].w = _viewProjection[3][3] - _viewProjection[3][2];

	// normalize
	for (unsigned int i = 0; i < 6; ++i)
	{
		float len = glm::length(glm::vec3(planes[i]));
		assert(len != 0.0f);
		planes[i] /= len;
	}
}

// code based on http://www.flipcode.com/archives/Frustum_Culling.shtml
bool Frustum::testSphere(const glm::vec4 &_sphere) const
{
	// various distances
	float dist;
	int inside = 0;

	// calculate our distances to each of the planes
	for (int i = 0; i < 6; ++i) 
	{
		// find the distance to this plane
		dist = glm::dot(glm::vec3(planes[i]), glm::vec3(_sphere)) + planes[i].w;

		// if this distance is < -sphere.radius, we are outside
		if (dist < -_sphere.w)
		{
			return false;
		}

		// else if the distance is between +- radius, then we intersect the plane
		if (fabs(dist) < _sphere.w)
		{
			++inside;
		}	
		if (inside > 2)
		{
			return true;
		}
	}

	// otherwise we are fully in view
	return true;
}
