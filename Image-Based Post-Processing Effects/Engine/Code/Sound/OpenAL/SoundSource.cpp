#include "SoundSource.h"
#include ".\..\..\Utilities\Utility.h"
#include "SoundBuffer.h"

const glm::vec3 SoundSource::defaultPosition;
const float SoundSource::defaultVolume = 1.0f;

std::shared_ptr<SoundSource> SoundSource::createSoundSource(const glm::vec3 *_position, const float *_volume, const float *_soundTypeVolume, bool _looping, bool _relative)
{
	return std::shared_ptr<SoundSource>(new SoundSource(_position, _volume, _soundTypeVolume, _looping, _relative));
}

SoundSource::~SoundSource()
{
	stop();
	alDeleteSources(1, &sourceId);
}

void SoundSource::play()
{
	alSourcePlay(sourceId);
}

void SoundSource::pause()
{
	alSourcePause(sourceId);
}

void SoundSource::stop()
{
	alSourceStop(sourceId);
}

void SoundSource::setBuffer(const std::shared_ptr<SoundBuffer> &_buffer)
{
	stop();
	soundBuffer = _buffer;
	if (soundBuffer && soundBuffer->isValid())
	{
		alSourcei(sourceId, AL_BUFFER, soundBuffer->getBufferId());
		isBufferSet = true;
	}
	else
	{
		isBufferSet = false;
	}
}

void SoundSource::setSpeed(const glm::vec3 &_speed)
{
	alSource3f(sourceId, AL_VELOCITY, _speed.x, _speed.y, _speed.z);
}

void SoundSource::setProperty(int _parameter, const ALfloat &_value)
{
	alSourcef(sourceId, _parameter, _value);
}

void SoundSource::update()
{
	if (!isBufferSet && soundBuffer && soundBuffer->isValid())
	{
		alSourcei(sourceId, AL_BUFFER, soundBuffer->getBufferId());
		isBufferSet = true;
	}
	alSource3f(sourceId, AL_POSITION, position->x, position->y, position->z);
	alSourcef(sourceId, AL_GAIN, (soundTypeVolume ? *soundTypeVolume : defaultVolume) * (volume ? *volume : defaultVolume));;
}

bool SoundSource::isPlaying()
{
	ALint value;
	alGetSourcei(sourceId, AL_SOURCE_STATE, &value); 
	return value == AL_PLAYING;
}

bool SoundSource::isWaiting()
{
	return !isBufferSet;
}

SoundSource::SoundSource(const glm::vec3 *_position, const float *_volume, const float *_soundTypeVolume, bool _looping, bool _relative)
	:position(_position), volume(_volume), soundTypeVolume(_soundTypeVolume), soundBuffer(nullptr), isBufferSet(false)
{
	alGenSources(1, &sourceId);

	if (!position)
	{
		position = &defaultPosition;
	}

	if (_looping)
	{
		alSourcei(sourceId, AL_LOOPING, AL_TRUE);
	}
	if (_relative)
	{
		alSourcei(sourceId, AL_SOURCE_RELATIVE, AL_TRUE);
	}
	update();
}
