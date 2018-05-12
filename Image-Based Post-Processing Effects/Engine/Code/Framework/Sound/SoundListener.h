#pragma once
#include <al.h>
#include <glm\vec3.hpp>

class SoundListener
{
public:
	explicit SoundListener(const glm::vec3 &_position = glm::vec3());
	void setSpeed(const glm::vec3 &_speed);
	void setPosition(const glm::vec3 &_position);
	void setOrientation(const glm::vec3 &_at, const glm::vec3 &_up);
	void updatePosition();
	void setGain(const ALfloat &_gain);

private:
	glm::vec3 position;
};