#pragma once
#include <al.h>
#include <string>
#include <memory>
#include <glm\vec3.hpp>

class SoundBuffer;

class SoundSource
{
public:
	static std::shared_ptr<SoundSource> createSoundSource(const glm::vec3 *_position = nullptr, const float *_volume = nullptr, const float *_soundTypeVolume = nullptr, const bool &_looping = false, const bool &_relative = false);
	
	SoundSource(const SoundSource &) = delete;
	SoundSource(const SoundSource &&) = delete;
	SoundSource &operator= (const SoundSource &) = delete;
	SoundSource &operator= (const SoundSource &&) = delete;
	~SoundSource();
	void play();
	void pause();
	void stop();
	void setBuffer(const std::shared_ptr<SoundBuffer> &_buffer);
	void setSpeed(const glm::vec3 &_speed);
	void setProperty(const int &_parameter, const ALfloat &_value);
	void update();
	bool isPlaying();
	bool isWaiting();

private:
	ALuint sourceId;
	const float *soundTypeVolume;
	static const glm::vec3 defaultPosition;
	static const float defaultVolume;
	const glm::vec3 *position;
	const float *volume;
	std::shared_ptr<SoundBuffer> soundBuffer;
	bool isBufferSet;

	explicit SoundSource(const glm::vec3 *_position = nullptr, const float *_volume = nullptr, const float *_soundTypeVolume = nullptr, const bool &_looping = false, const bool &_relative = false);
};