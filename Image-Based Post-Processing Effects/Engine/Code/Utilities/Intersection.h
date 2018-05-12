#pragma once
#include <glm\vec3.hpp>
#include <glm\vec2.hpp>
#include <glm\gtc\quaternion.hpp>
#include <vector>
#include <memory>

class Camera;
class Window;
struct Entity;

bool intersectRayAabb(const glm::vec3 &_origin, const glm::vec3 &_dir, const glm::vec3 &_maxCorner, glm::vec2 &_result, const glm::vec3 &_position = glm::vec3());

bool intersectRayObb(const glm::vec3 &_origin, const glm::vec3 &_dir, const glm::vec3 &_maxCorner, const glm::vec3 &_position, const glm::quat &_rotation, glm::vec2 &_result);

const Entity *getSelectedEntity(const std::vector<const Entity *> &_entities, const glm::vec3 &_cameraPosition, const glm::vec3 &_mouseDirection, const bool &_oriented);

glm::vec3 getMouseDirection(const std::shared_ptr<Window> &, Camera *_camera);