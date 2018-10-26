#include "SoundSystem.h"
#include ".\..\SystemManager.h"
#include ".\..\EntityManager.h"
#include "Sound\OpenAL\SoundFramework.h"
#include "Window\Window.h"
#include "Graphics\Camera.h"
#include "Level.h"

SoundSystem::SoundSystem()
	:m_soundFramework(new SoundFramework()),
	m_entityManager(EntityManager::getInstance())
{
	m_validBitMaps.push_back(Component<SoundComponent>::getTypeId());
}

SoundSystem::~SoundSystem()
{
	delete m_soundFramework;
}

void SoundSystem::init()
{
	m_entityManager.addOnComponentAddedListener(this);
	m_entityManager.addOnEntityDestructionListener(this);
	m_entityManager.addOnComponentRemovedListener(this);

	m_soundFramework->init();

	SettingsManager &settingsManager = SettingsManager::getInstance();

	m_masterVolume = settingsManager.getDoubleSetting("sound", "master_volume", 0.5);
	m_masterVolume->addListener([&](double _value) { m_soundFramework->setMasterVolume((float)_value); });
	m_soundFramework->setMasterVolume((float)m_masterVolume->get());

	m_musicVolume = settingsManager.getDoubleSetting("sound", "music_volume", 0.5);
	m_musicVolume->addListener([&](double _value) { m_soundFramework->setVolume(SoundType::MUSIC, (float)_value); });
	m_soundFramework->setVolume(SoundType::MUSIC, (float)m_musicVolume->get());

	m_effectVolume = settingsManager.getDoubleSetting("sound", "effect_volume", 0.5);
	m_effectVolume->addListener([&](double _value) { m_soundFramework->setVolume(SoundType::EFFECT, (float)_value); });
	m_soundFramework->setVolume(SoundType::EFFECT, (float)m_effectVolume->get());

	m_uiVolume = settingsManager.getDoubleSetting("sound", "ui_volume", 0.5);
	m_uiVolume->addListener([&](double _value) { m_soundFramework->setVolume(SoundType::UI, (float)_value); });
	m_soundFramework->setVolume(SoundType::UI, (float)m_uiVolume->get());

	settingsManager.saveToIni();
}

void SoundSystem::input(double _currentTime, double _timeDelta)
{
}

void SoundSystem::update(double _currentTime, double _timeDelta)
{
	std::shared_ptr<Level> level = SystemManager::getInstance().getLevel();
	if (!level || level->m_cameras.empty())
	{
		// we cant do play any (directional) sound if there is no active camera
		return;
	}

	std::shared_ptr<Camera> camera = level->m_cameras[level->m_activeCameraIndex];

	std::vector<const Entity *> entitiesToRemove = m_soundFramework->update(camera);
	for (const Entity *entity : entitiesToRemove)
	{
		m_managedEntities.erase(std::remove(m_managedEntities.begin(), m_managedEntities.end(), entity), m_managedEntities.end());
	}
	for (const Entity *entity : m_managedEntities)
	{
		SoundComponent *sc = m_entityManager.getComponent<SoundComponent>(entity);
		m_soundFramework->setPaused(entity, sc->m_paused);
	}
}

void SoundSystem::render()
{
}

void SoundSystem::onComponentAdded(const Entity *_entity, BaseComponent *_addedComponent)
{
	if (validate(m_entityManager.getComponentBitField(_entity)) && std::find(m_managedEntities.begin(), m_managedEntities.end(), _entity) == m_managedEntities.end())
	{
		m_managedEntities.push_back(_entity);
		SoundComponent *sc = m_entityManager.getComponent<SoundComponent>(_entity);
		TransformationComponent *tc = m_entityManager.getComponent<TransformationComponent>(_entity);

		m_soundFramework->addSound(_entity, sc->m_soundFile, sc->m_soundType, &sc->m_volume, sc->m_looping, sc->m_soundType != SoundType::EFFECT, sc->m_paused, tc ? &tc->m_position : nullptr, sc->m_loadInstantly);
	}
}

void SoundSystem::onComponentRemoved(const Entity *_entity, BaseComponent *_removedComponent)
{
	if (!validate(m_entityManager.getComponentBitField(_entity)) && std::find(m_managedEntities.begin(), m_managedEntities.end(), _entity) != m_managedEntities.end())
	{
		m_soundFramework->removeSound(_entity);
		m_managedEntities.erase(std::remove(m_managedEntities.begin(), m_managedEntities.end(), _entity), m_managedEntities.end());
	}
}

void SoundSystem::onDestruction(const Entity *_entity)
{
	if (std::find(m_managedEntities.begin(), m_managedEntities.end(), _entity) != m_managedEntities.end())
	{
		m_soundFramework->removeSound(_entity);
		m_managedEntities.erase(std::remove(m_managedEntities.begin(), m_managedEntities.end(), _entity), m_managedEntities.end());
	}
}

bool SoundSystem::validate(std::uint64_t _bitMap)
{
	for (std::uint64_t configuration : m_validBitMaps)
	{
		if ((configuration & _bitMap) == configuration)
		{
			return true;
		}
	}
	return false;
}
