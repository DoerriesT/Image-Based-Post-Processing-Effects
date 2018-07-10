#pragma once
#include <alc.h>
#include <glm\vec3.hpp>
#include <string>
#include <vector>
#include <map>
#include <set>
#include ".\..\..\EntityComponentSystem\EntityManager.h"
#include "SoundListener.h"


class Camera;
class SoundSource;

class SoundFramework
{
public:
	explicit SoundFramework();
	SoundFramework(const SoundFramework &) = delete;
	SoundFramework(const SoundFramework &&) = delete;
	SoundFramework &operator= (const SoundFramework &) = delete;
	SoundFramework &operator= (const SoundFramework &&) = delete;
	~SoundFramework();
	void init();
	void addSound(const Entity *_entity, const std::string &_file, const SoundType &_soundType, const float *_volume = nullptr, bool _looping = false, bool _relative = false, bool _paused = true, const glm::vec3 *_position = nullptr, bool _loadInstantly = false);
	void removeSound(const Entity *_entity);
	std::vector<const Entity *> update(const std::shared_ptr<Camera> &_camera);
	void setPaused(const Entity *_entity, bool _paused);
	void setVolume(const SoundType &_soundType, float _volume);
	void setMasterVolume(float _volume);
	void setAttenuationModel(int _model);


private:
	ALCdevice *device;
	ALCcontext *context;
	SoundListener soundListener;
	std::map<const Entity *, std::shared_ptr<SoundSource>> soundSourceMap;
	std::map<const Entity *, SoundType> entitySoundTypeMap;
	std::set<const Entity *> pausedEntities;
	float musicVolume = 1.0f;
	float effectVolume = 1.0f;
	float uiVolume = 1.0f;

	void addSoundSource(const Entity *_entity, const glm::vec3 *_position, const float *_volume, const float *_soundTypeVolume, bool _looping, bool _relative);
};