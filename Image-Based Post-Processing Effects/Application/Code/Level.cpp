#include "Level.h"
#include "EntityComponentSystem\EntityManager.h"
#include <glm\ext.hpp>
#include <Graphics\Mesh.h>
#include <Graphics\EntityRenderData.h>
#include <Framework\Graphics\RenderData.h>
#include "EasingFunctions.h"
#include <Engine.h>
#include <Graphics\Material.h>
#include <Graphics\Texture.h>
#include <Graphics\Camera.h>

std::shared_ptr<Level> App::loadLevel()
{
	EntityManager &entityManager = EntityManager::getInstance();
	std::shared_ptr<Level> level = std::make_shared<Level>();

	// filepath
	level->filepath = "Resources/Levels/default/";

	// camera(s)
	std::shared_ptr<Camera> camera0 = std::make_shared<Camera>(glm::vec3(0.0f, 7.5f, 11.0f), glm::quat(glm::vec3(glm::radians(35.0f), 0.0f, 0.0f)));
	
	level->cameras.push_back(camera0);
	level->activeCameraIndex = 0;

	// exposure
	level->exposure = 16.0f;

	// water
	level->water.enabled = false;
	level->water.level = -6.0;

	// sun
	level->sun.direction = glm::normalize(glm::vec3(1.0f, 1.0f, -1.0f));

	/*- set skybox entity and environment maps*/
	const Entity *skyboxEntity = entityManager.createEntity();
	level->environment.skyboxEntity = skyboxEntity;

	//oceanLevel->environment.environmentMap = Texture::createTexture("Resources/Textures/oceanskybox.dds", true);
	if (SettingsManager::getInstance().getBoolSetting("graphics", "load_environment_from_file", true)->get())
	{
		level->environment.environmentProbe = EnvironmentProbe::createEnvironmentProbe(glm::vec3(0.0f, 10.0f, 0.0f), Texture::createTexture(level->filepath + "reflectance.dds", true), Texture::createTexture(level->filepath + "irradiance.dds", true));
	}
	else
	{
		level->environment.environmentProbe = EnvironmentProbe::createEnvironmentProbe(glm::vec3(0.0f, 10.0f, 0.0f));
	}
	level->environment.brdfMap = Texture::createTexture("Resources/Textures/brdf.dds", true);

	AtmosphereParams params;
	params.intensity = glm::vec3(1.0f, 1.0f, 1.0f);
	params.lightDir = level->sun.direction;
	params.mieBrightness = 100.0f;
	params.mieCollectionPower = 0.39f;
	params.mieDistribution = 63.0f;
	params.mieStrength = 264.0f;
	params.rayleighBrightness = 33.0f;
	params.rayleighCollectionPower = 0.81f;
	params.rayleighStrength = 139.0f;
	params.scatterStrength = 28.0f;
	params.spotBrightness = 1000.0f;
	params.stepCount = 16;
	params.surfaceHeight = 0.99f;

	level->environment.useAtmosphere = true;
	level->environment.isAtmosphereValid = false;
	level->environment.atmosphereParams = params;

	/*- set the lights in the scene*/
	level->lights.directionalLights.push_back(DirectionalLight::createDirectionalLight(glm::vec3(1.0f, 1.0f, 1.0f), level->sun.direction, true));

	/*- set music*/
	/*const Entity *musicEntity = entityManager.createEntity();
	level->entityMap["music"] = musicEntity;
	entityManager.addComponent<SoundComponent>(musicEntity, "Resources/Sounds/Gymnopedie No 1.ogg", SoundType::MUSIC, 1.0f, true, false);

	const Entity *ambienceEntity = entityManager.createEntity();
	level->entityMap["ambience"] = ambienceEntity;
	entityManager.addComponent<SoundComponent>(ambienceEntity, "Resources/Sounds/Ocean_Waves-Mike_Koenig.ogg", SoundType::EFFECT, 0.25f, true, false);*/

	level->id = (size_t)0;
	level->valid = true;
	level->name = "default";

	return level;
}
