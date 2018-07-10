#include "SoundSystem.h"
#include ".\..\SystemManager.h"
#include ".\..\EntityManager.h"
#include "Sound\OpenAL\SoundFramework.h"
#include "Window\Window.h"
#include "Graphics\Camera.h"
#include "Level.h"

SoundSystem::SoundSystem()
	:soundFramework(new SoundFramework()),
	entityManager(EntityManager::getInstance())
{
	validBitMaps.push_back(Component<SoundComponent>::getTypeId());
}

SoundSystem::~SoundSystem()
{
	delete soundFramework;
}

void SoundSystem::init()
{
	entityManager.addOnComponentAddedListener(this);
	entityManager.addOnEntityDestructionListener(this);
	entityManager.addOnComponentRemovedListener(this);

	soundFramework->init();

	SettingsManager &settingsManager = SettingsManager::getInstance();

	masterVolume = settingsManager.getDoubleSetting("sound", "master_volume", 0.5);
	masterVolume->addListener([&](double _value) { soundFramework->setMasterVolume((float)_value); });
	soundFramework->setMasterVolume((float)masterVolume->get());

	musicVolume = settingsManager.getDoubleSetting("sound", "music_volume", 0.5);
	musicVolume->addListener([&](double _value) { soundFramework->setVolume(SoundType::MUSIC, (float)_value); });
	soundFramework->setVolume(SoundType::MUSIC, (float)musicVolume->get());

	effectVolume = settingsManager.getDoubleSetting("sound", "effect_volume", 0.5);
	effectVolume->addListener([&](double _value) { soundFramework->setVolume(SoundType::EFFECT, (float)_value); });
	soundFramework->setVolume(SoundType::EFFECT, (float)effectVolume->get());

	uiVolume = settingsManager.getDoubleSetting("sound", "ui_volume", 0.5);
	uiVolume->addListener([&](double _value) { soundFramework->setVolume(SoundType::UI, (float)_value); });
	soundFramework->setVolume(SoundType::UI, (float)uiVolume->get());

	settingsManager.saveToIni();
}

void SoundSystem::input(double _currentTime, double _timeDelta)
{
}

void SoundSystem::update(double _currentTime, double _timeDelta)
{
	std::shared_ptr<Level> level = SystemManager::getInstance().getLevel();
	if (!level || level->cameras.empty())
	{
		// we cant do play any (directional) sound if there is no active camera
		return;
	}

	std::shared_ptr<Camera> camera = level->cameras[level->activeCameraIndex];

	std::vector<const Entity *> entitiesToRemove = soundFramework->update(camera);
	for (const Entity *entity : entitiesToRemove)
	{
		managedEntities.erase(std::remove(managedEntities.begin(), managedEntities.end(), entity), managedEntities.end());
	}
	for (const Entity *entity : managedEntities)
	{
		SoundComponent *sc = entityManager.getComponent<SoundComponent>(entity);
		soundFramework->setPaused(entity, sc->paused);
	}
}

void SoundSystem::render()
{
}

void SoundSystem::onComponentAdded(const Entity *_entity, BaseComponent *_addedComponent)
{
	if (validate(entityManager.getComponentBitField(_entity)) && std::find(managedEntities.begin(), managedEntities.end(), _entity) == managedEntities.end())
	{
		managedEntities.push_back(_entity);
		SoundComponent *sc = entityManager.getComponent<SoundComponent>(_entity);
		TransformationComponent *tc = entityManager.getComponent<TransformationComponent>(_entity);

		soundFramework->addSound(_entity, sc->soundFile, sc->soundType, &sc->volume, sc->looping, sc->soundType != SoundType::EFFECT, sc->paused, tc ? &tc->position : nullptr, sc->loadInstantly);
	}
}

void SoundSystem::onComponentRemoved(const Entity *_entity, BaseComponent *_removedComponent)
{
	if (!validate(entityManager.getComponentBitField(_entity)) && std::find(managedEntities.begin(), managedEntities.end(), _entity) != managedEntities.end())
	{
		soundFramework->removeSound(_entity);
		managedEntities.erase(std::remove(managedEntities.begin(), managedEntities.end(), _entity), managedEntities.end());
	}
}

void SoundSystem::onDestruction(const Entity *_entity)
{
	if (std::find(managedEntities.begin(), managedEntities.end(), _entity) != managedEntities.end())
	{
		soundFramework->removeSound(_entity);
		managedEntities.erase(std::remove(managedEntities.begin(), managedEntities.end(), _entity), managedEntities.end());
	}
}

bool SoundSystem::validate(std::uint64_t _bitMap)
{
	for (std::uint64_t configuration : validBitMaps)
	{
		if ((configuration & _bitMap) == configuration)
		{
			return true;
		}
	}
	return false;
}
