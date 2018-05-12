#include "SoundFramework.h"
#include <iostream>
#include ".\..\..\Utilities\Utility.h"
#include ".\..\..\Graphics\Camera.h"
#include "SoundBuffer.h"
#include "SoundSource.h"

SoundFramework::SoundFramework()
{
}

void SoundFramework::addSoundSource(const Entity *_entity, const glm::vec3 *_position, const float *_volume, const float *_soundTypeVolume, const bool &_looping, const bool &_relative)
{
	if (soundSourceMap.find(_entity) != soundSourceMap.end())
	{
		remove(soundSourceMap, _entity);
	}
	soundSourceMap.emplace(_entity, SoundSource::createSoundSource(_position, _volume, _soundTypeVolume, _looping, _relative));
}

SoundFramework::~SoundFramework()
{
	if (context)
	{
		alcDestroyContext(context);
	}
	if (device)
	{
		alcCloseDevice(device);
	}
}

void SoundFramework::init()
{
	device = alcOpenDevice(nullptr);
	if (!device)
	{
		std::cout << "Failed to open the default OpenAL device." << std::endl;
	}
	context = alcCreateContext(device, nullptr);
	alcMakeContextCurrent(context);

	alGetError(); // clear error code

	setAttenuationModel(AL_EXPONENT_DISTANCE);
	/*soundListener.setPosition(glm::vec3(0.0f));
	soundListener.setOrientation(glm::vec3(0.0f), glm::vec3(0.0f));*/
}

void SoundFramework::addSound(const Entity *_entity, const std::string &_file, const SoundType &_soundType, const float *_volume, const bool &_looping, const bool &_relative, const bool &_paused, const glm::vec3 *_position, const bool &_loadInstantly)
{
	switch (_soundType)
	{
	case SoundType::MUSIC:
	{
		addSoundSource(_entity, _position, _volume, &musicVolume, _looping, _relative);
		break;
	}
	case SoundType::EFFECT:
	{
		addSoundSource(_entity, _position, _volume, &effectVolume, _looping, _relative);
		break;
	}
	case SoundType::UI:
	{
		addSoundSource(_entity, _position, _volume, &uiVolume, _looping, _relative);
		break;
	}
	default:
	{
		std::cout << "Missing SoundType!" << std::endl;
	}
	}

	std::shared_ptr<SoundSource> soundSource = soundSourceMap[_entity];
	soundSource->setBuffer(SoundBuffer::createSoundBuffer(_file, _loadInstantly));

	entitySoundTypeMap.emplace(_entity, _soundType);
	if (_paused)
	{
		pausedEntities.emplace(_entity);
		soundSource->pause();
	}
	else
	{
		soundSource->play();
	}
}

void SoundFramework::removeSound(const Entity *_entity)
{
	assert(soundSourceMap.find(_entity) != soundSourceMap.end());
	std::shared_ptr<SoundSource> soundSource = soundSourceMap[_entity];
	soundSource->stop();
	soundSource.reset();
	entitySoundTypeMap.erase(_entity);
	soundSourceMap.erase(_entity);
}

std::vector<const Entity*> SoundFramework::update(const std::shared_ptr<Camera> &_camera)
{
	soundListener.setOrientation(_camera->getForwardDirection(), _camera->getUpDirection());
	soundListener.setPosition(_camera->getPosition());
	
	std::vector<const Entity *> readyToRemove;
	for(auto iter = soundSourceMap.begin(); iter != soundSourceMap.end(); ++iter)
	{
		std::shared_ptr<SoundSource> soundSource = iter->second;
		if (!soundSource->isPlaying() && !soundSource->isWaiting() && pausedEntities.find(iter->first) == pausedEntities.end())
		{
			readyToRemove.push_back(iter->first);
			continue;
		}
		soundSource->update();
	}
	for (const Entity *entity : readyToRemove)
	{
		soundSourceMap.erase(entity);
		entitySoundTypeMap.erase(entity);
	}

#ifdef _DEBUG
	alErrorCheck("SoundFramework::update Error");
#endif // _DEBUG

	return readyToRemove;
}


void SoundFramework::setPaused(const Entity *_entity, const bool &_paused)
{
	assert(soundSourceMap.find(_entity) != soundSourceMap.end());
	std::shared_ptr<SoundSource> soundSource = soundSourceMap[_entity];
	if (_paused && soundSource->isPlaying())
	{
		pausedEntities.emplace(_entity);
		soundSource->pause();
	}
	else if (!_paused && !soundSource->isPlaying())
	{
		pausedEntities.erase(_entity);
		soundSource->play();
	}
}

void SoundFramework::setVolume(const SoundType &_soundType, const float &_volume)
{
	switch (_soundType)
	{
	case SoundType::MUSIC:
	{
		musicVolume = _volume;
		break;
	}
	case SoundType::EFFECT:
	{
		effectVolume = _volume;
		break;
	}
	case SoundType::UI:
	{
		uiVolume = _volume;
		break;
	}
	default:
	{
		std::cout << "Missing SoundType!" << std::endl;
	}
	}
}

void SoundFramework::setMasterVolume(const float &_volume)
{
	soundListener.setGain(_volume);
}

void SoundFramework::setAttenuationModel(const int &_model)
{
	alDistanceModel(_model);
}


