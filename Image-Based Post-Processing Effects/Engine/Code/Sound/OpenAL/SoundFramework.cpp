#include "SoundFramework.h"
#include <iostream>
#include "Utilities\ContainerUtility.h"
#include "ALUtility.h"
#include "Graphics\Camera.h"
#include "SoundBuffer.h"
#include "SoundSource.h"

SoundFramework::SoundFramework()
{
}

void SoundFramework::addSoundSource(const Entity *_entity, const glm::vec3 *_position, const float *_volume, const float *_soundTypeVolume, bool _looping, bool _relative)
{
	if (m_soundSourceMap.find(_entity) != m_soundSourceMap.end())
	{
		ContainerUtility::remove(m_soundSourceMap, _entity);
	}
	m_soundSourceMap.emplace(_entity, SoundSource::createSoundSource(_position, _volume, _soundTypeVolume, _looping, _relative));
}

SoundFramework::~SoundFramework()
{
	if (m_context)
	{
		alcDestroyContext(m_context);
	}
	if (m_device)
	{
		alcCloseDevice(m_device);
	}
}

void SoundFramework::init()
{
	m_device = alcOpenDevice(nullptr);
	if (!m_device)
	{
		std::cout << "Failed to open the default OpenAL device." << std::endl;
	}
	m_context = alcCreateContext(m_device, nullptr);
	alcMakeContextCurrent(m_context);

	alGetError(); // clear error code

	setAttenuationModel(AL_EXPONENT_DISTANCE);
	/*soundListener.setPosition(glm::vec3(0.0f));
	soundListener.setOrientation(glm::vec3(0.0f), glm::vec3(0.0f));*/
}

void SoundFramework::addSound(const Entity *_entity, const std::string &_file, const SoundType &_soundType, const float *_volume, bool _looping, bool _relative, bool _paused, const glm::vec3 *_position, bool _loadInstantly)
{
	switch (_soundType)
	{
	case SoundType::MUSIC:
	{
		addSoundSource(_entity, _position, _volume, &m_musicVolume, _looping, _relative);
		break;
	}
	case SoundType::EFFECT:
	{
		addSoundSource(_entity, _position, _volume, &m_effectVolume, _looping, _relative);
		break;
	}
	case SoundType::UI:
	{
		addSoundSource(_entity, _position, _volume, &m_uiVolume, _looping, _relative);
		break;
	}
	default:
	{
		std::cout << "Missing SoundType!" << std::endl;
	}
	}

	std::shared_ptr<SoundSource> soundSource = m_soundSourceMap[_entity];
	soundSource->setBuffer(SoundBuffer::createSoundBuffer(_file, _loadInstantly));

	m_entitySoundTypeMap.emplace(_entity, _soundType);
	if (_paused)
	{
		m_pausedEntities.emplace(_entity);
		soundSource->pause();
	}
	else
	{
		soundSource->play();
	}
}

void SoundFramework::removeSound(const Entity *_entity)
{
	assert(m_soundSourceMap.find(_entity) != m_soundSourceMap.end());
	std::shared_ptr<SoundSource> soundSource = m_soundSourceMap[_entity];
	soundSource->stop();
	soundSource.reset();
	m_entitySoundTypeMap.erase(_entity);
	m_soundSourceMap.erase(_entity);
}

std::vector<const Entity*> SoundFramework::update(const std::shared_ptr<Camera> &_camera)
{
	m_soundListener.setOrientation(_camera->getForwardDirection(), _camera->getUpDirection());
	m_soundListener.setPosition(_camera->getPosition());
	
	std::vector<const Entity *> readyToRemove;
	for(auto iter = m_soundSourceMap.begin(); iter != m_soundSourceMap.end(); ++iter)
	{
		std::shared_ptr<SoundSource> soundSource = iter->second;
		if (!soundSource->isPlaying() && !soundSource->isWaiting() && m_pausedEntities.find(iter->first) == m_pausedEntities.end())
		{
			readyToRemove.push_back(iter->first);
			continue;
		}
		soundSource->update();
	}
	for (const Entity *entity : readyToRemove)
	{
		m_soundSourceMap.erase(entity);
		m_entitySoundTypeMap.erase(entity);
	}

#ifdef _DEBUG
	ALUtility::alErrorCheck("SoundFramework::update Error");
#endif // _DEBUG

	return readyToRemove;
}


void SoundFramework::setPaused(const Entity *_entity, bool _paused)
{
	assert(m_soundSourceMap.find(_entity) != m_soundSourceMap.end());
	std::shared_ptr<SoundSource> soundSource = m_soundSourceMap[_entity];
	if (_paused && soundSource->isPlaying())
	{
		m_pausedEntities.emplace(_entity);
		soundSource->pause();
	}
	else if (!_paused && !soundSource->isPlaying())
	{
		m_pausedEntities.erase(_entity);
		soundSource->play();
	}
}

void SoundFramework::setVolume(const SoundType &_soundType, float _volume)
{
	switch (_soundType)
	{
	case SoundType::MUSIC:
	{
		m_musicVolume = _volume;
		break;
	}
	case SoundType::EFFECT:
	{
		m_effectVolume = _volume;
		break;
	}
	case SoundType::UI:
	{
		m_uiVolume = _volume;
		break;
	}
	default:
	{
		std::cout << "Missing SoundType!" << std::endl;
	}
	}
}

void SoundFramework::setMasterVolume(float _volume)
{
	m_soundListener.setGain(_volume);
}

void SoundFramework::setAttenuationModel(int _model)
{
	alDistanceModel(_model);
}


