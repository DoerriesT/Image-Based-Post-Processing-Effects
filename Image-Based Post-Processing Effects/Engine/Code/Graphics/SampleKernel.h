#pragma once
#include <glm/vec2.hpp>

float ngon(float theta, float n);
glm::vec2 shirleyUnitSquareToDisk(const glm::vec2 &_point, bool ngonWarp = false, float blades = 6.0f);