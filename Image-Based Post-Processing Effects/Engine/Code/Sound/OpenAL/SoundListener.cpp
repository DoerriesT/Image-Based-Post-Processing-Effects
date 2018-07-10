#include "SoundListener.h"

SoundListener::SoundListener(const glm::vec3 &_position)
{
	position = _position;
	updatePosition();
	alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
}

void SoundListener::setSpeed(const glm::vec3 &_speed)
{
	alListener3f(AL_VELOCITY, _speed.x, _speed.y, _speed.z);
}

void SoundListener::setPosition(const glm::vec3 &_position)
{
	position = _position;
	updatePosition();
}

void SoundListener::setOrientation(const glm::vec3 &_at, const glm::vec3 &_up)
{
	ALfloat data[6];
	data[0] = _at.x;
	data[1] = _at.y;
	data[2] = _at.z;
	data[3] = _up.x;
	data[4] = _up.y;
	data[5] = _up.z;
	alListenerfv(AL_ORIENTATION, data);
}

void SoundListener::updatePosition()
{
	alListener3f(AL_POSITION, position.x, position.y, position.z);
}

void SoundListener::setGain(const ALfloat &_gain)
{
	alListenerf(AL_GAIN, _gain);
}
