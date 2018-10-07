#pragma once
#include <al.h>
#include <string>
#include <memory>
#include <glm\vec3.hpp>

class SoundBuffer;

class SoundSource
{
public:
	static std::shared_ptr<SoundSource> createSoundSource(const glm::vec3 *_position = nullptr, const float *_volume = nullptr, const float *_soundTypeVolume = nullptr, bool _looping = false, bool _relative = false);
	
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
	void setProperty(int _parameter, const ALfloat &_value);
	void update();
	bool isPlaying();
	bool isWaiting();

private:
	static const glm::vec3 defaultPosition;
	const glm::vec3 *position;
	ALuint sourceId;
	std::shared_ptr<SoundBuffer> soundBuffer;
	const float *soundTypeVolume;
	static const float defaultVolume;
	const float *volume;
	bool isBufferSet;

	explicit SoundSource(const glm::vec3 *_position = nullptr, const float *_volume = nullptr, const float *_soundTypeVolume = nullptr, bool _looping = false, bool _relative = false);
};