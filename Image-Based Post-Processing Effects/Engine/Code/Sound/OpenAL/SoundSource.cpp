#include "SoundSource.h"
#include ".\..\..\Utilities\Utility.h"
#include "SoundBuffer.h"

const glm::vec3 SoundSource::m_defaultPosition;
const float SoundSource::m_defaultVolume = 1.0f;

std::shared_ptr<SoundSource> SoundSource::createSoundSource(const glm::vec3 *_position, const float *_volume, const float *_soundTypeVolume, bool _looping, bool _relative)
{
	return std::shared_ptr<SoundSource>(new SoundSource(_position, _volume, _soundTypeVolume, _looping, _relative));
}

SoundSource::~SoundSource()
{
	stop();
	alDeleteSources(1, &m_sourceId);
}

void SoundSource::play()
{
	alSourcePlay(m_sourceId);
}

void SoundSource::pause()
{
	alSourcePause(m_sourceId);
}

void SoundSource::stop()
{
	alSourceStop(m_sourceId);
}

void SoundSource::setBuffer(const std::shared_ptr<SoundBuffer> &_buffer)
{
	stop();
	m_soundBuffer = _buffer;
	if (m_soundBuffer && m_soundBuffer->isValid())
	{
		alSourcei(m_sourceId, AL_BUFFER, m_soundBuffer->getBufferId());
		m_isBufferSet = true;
	}
	else
	{
		m_isBufferSet = false;
	}
}

void SoundSource::setSpeed(const glm::vec3 &_speed)
{
	alSource3f(m_sourceId, AL_VELOCITY, _speed.x, _speed.y, _speed.z);
}

void SoundSource::setProperty(int _parameter, const ALfloat &_value)
{
	alSourcef(m_sourceId, _parameter, _value);
}

void SoundSource::update()
{
	if (!m_isBufferSet && m_soundBuffer && m_soundBuffer->isValid())
	{
		alSourcei(m_sourceId, AL_BUFFER, m_soundBuffer->getBufferId());
		m_isBufferSet = true;
	}
	alSource3f(m_sourceId, AL_POSITION, m_position->x, m_position->y, m_position->z);
	alSourcef(m_sourceId, AL_GAIN, (m_soundTypeVolume ? *m_soundTypeVolume : m_defaultVolume) * (m_volume ? *m_volume : m_defaultVolume));;
}

bool SoundSource::isPlaying()
{
	ALint value;
	alGetSourcei(m_sourceId, AL_SOURCE_STATE, &value); 
	return value == AL_PLAYING;
}

bool SoundSource::isWaiting()
{
	return !m_isBufferSet;
}

SoundSource::SoundSource(const glm::vec3 *_position, const float *_volume, const float *_soundTypeVolume, bool _looping, bool _relative)
	:m_position(_position), m_volume(_volume), m_soundTypeVolume(_soundTypeVolume), m_soundBuffer(nullptr), m_isBufferSet(false)
{
	alGenSources(1, &m_sourceId);

	if (!m_position)
	{
		m_position = &m_defaultPosition;
	}

	if (_looping)
	{
		alSourcei(m_sourceId, AL_LOOPING, AL_TRUE);
	}
	if (_relative)
	{
		alSourcei(m_sourceId, AL_SOURCE_RELATIVE, AL_TRUE);
	}
	update();
}
