#include "SampleKernel.h"
#include <glm/gtc/constants.hpp>

glm::vec2 shirleyUnitSquareToDisk(const glm::vec2 &_point, bool ngonWarp, float blades)
{
	float max_fstops = 8;
	float min_fstops = 1;
	float normalizedStops = 1.0f; //clamp_tpl((fstop - max_fstops) / (max_fstops - min_fstops), 0.0f, 1.0f);

	float phi;
	float r;
	const float a = 2 * _point.x - 1;
	const float b = 2 * _point.y - 1;
	if (abs(a) > abs(b)) // Use squares instead of absolute values
	{
		r = a;
		phi = (glm::pi<float>() / 4.0f) * (b / (a + 1e-6f));
	}
	else
	{
		r = b;
		phi = (glm::pi<float>() / 2.0f) - (glm::pi<float>() / 4.0f) * (a / (b + 1e-6f));
	}

	float rr = ngonWarp ? r * powf(ngon(phi, blades), normalizedStops) : r;
	rr = abs(rr) * (rr > 0.0f ? 1.0f : -1.0f);

	//normalizedStops *= -0.4f * PI;
	return glm::vec2(rr * cosf(phi + normalizedStops), rr * sinf(phi + normalizedStops));
}

float ngon(float theta, float n)
{
	return cosf(glm::pi<float>() / n) / cosf(theta - (2 * glm::pi<float>() / n) * floorf((n * theta + glm::pi<float>()) / (2 * glm::pi<float>())));
}
