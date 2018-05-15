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
#include <random>

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
	level->lights.directionalLights.push_back(DirectionalLight::createDirectionalLight(params.intensity, level->sun.direction, true));

	/*- set music*/
	/*const Entity *musicEntity = entityManager.createEntity();
	level->entityMap["music"] = musicEntity;
	entityManager.addComponent<SoundComponent>(musicEntity, "Resources/Sounds/Gymnopedie No 1.ogg", SoundType::MUSIC, 1.0f, true, false);

	const Entity *ambienceEntity = entityManager.createEntity();
	level->entityMap["ambience"] = ambienceEntity;
	entityManager.addComponent<SoundComponent>(ambienceEntity, "Resources/Sounds/Ocean_Waves-Mike_Koenig.ogg", SoundType::EFFECT, 0.25f, true, false);*/


	// objects
	{
		const Entity *planeEntity = entityManager.createEntity();
		level->entityMap["plane"] = planeEntity;
		entityManager.addComponent<ModelComponent>(planeEntity, std::vector<std::pair<std::string, Material>>({ std::make_pair("Resources/Models/plane.obj", Material(glm::vec4(glm::vec3(1.0f), 1.0f), 0.0f, 0.0f)) }));
		entityManager.addComponent<TransformationComponent>(planeEntity, glm::vec3(), glm::quat(glm::vec3(glm::radians(180.0f), 0.0f, 0.0f)), glm::vec3(100.0f));
		entityManager.addComponent<RenderableComponent>(planeEntity);

		std::default_random_engine e;
		std::uniform_real_distribution<float> dist(0.0f, 1.0f);
		std::uniform_real_distribution<float> dist1(1.0f, 3.0f);

		for (int i = 0; i < 10; ++i)
		{
			for (int j = 0; j < 10; ++j)
			{

				glm::vec3 color(dist(e), dist(e), dist(e));
				glm::vec3 position = glm::vec3(i - 5.0f, 0.0f, j - 5.0f) * 4.0f;
				glm::vec3 bouncePos = position + glm::vec3(0.0f, 10.0f, 0.0f);

				const Entity *teapotEntity = entityManager.createEntity();
				level->entityMap["teapot" + (i * 10 + j)] = teapotEntity;
				entityManager.addComponent<ModelComponent>(teapotEntity, std::vector<std::pair<std::string, Material>>({ std::make_pair("Resources/Models/teapot.obj", Material(glm::vec4(color, 1.0f), 0.0f, 0.0f)) }));
				entityManager.addComponent<TransformationComponent>(teapotEntity, position, glm::quat(glm::vec3(0.0, glm::radians(40.0f), 0.0f)), glm::vec3(1.0f));
				entityManager.addComponent<RenderableComponent>(teapotEntity);

				float speed = dist1(e) * 0.5f;

				std::vector<PathSegment> pathSegments;
				pathSegments.push_back(PathSegment(
					position,	// start pos
					bouncePos,	// end pos
					glm::vec3(0.0f, 1.0f, 0.0f),						// start tangent
					glm::vec3(0.0f, 1.0f, 0.0f),						// end tangent
					speed,														// duration
					linear));													// easing function
				pathSegments.push_back(PathSegment(
					bouncePos,
					position,
					glm::vec3(0.0f, -1.0f, 0.0),
					glm::vec3(0.0f, -1.0f, 0.0),
					speed,
					linear));
				entityManager.addComponent<MovementPathComponent>(teapotEntity, pathSegments, Engine::getCurrentTime(), true);
				entityManager.addComponent<PerpetualRotationComponent>(teapotEntity, glm::vec3(dist(e), dist(e), dist(e)));
			}
		}

		/*const Entity *teapotEntity = entityManager.createEntity();
		level->entityMap["teapot"] = teapotEntity;
		entityManager.addComponent<ModelComponent>(teapotEntity, std::vector<std::pair<std::string, Material>>({ std::make_pair("Resources/Models/teapot.obj", Material(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 0.0f, 0.0f)) }));
		auto *tc = entityManager.addComponent<TransformationComponent>(teapotEntity, glm::vec3(), glm::quat(glm::vec3(0.0, glm::radians(40.0f), 0.0f)), glm::vec3(1.0f));
		entityManager.addComponent<RenderableComponent>(teapotEntity);

		tc->vel = glm::vec2(1.0, 0.0);*/
	}

	level->id = (size_t)0;
	level->valid = true;
	level->name = "default";
	level->loaded = true;

	return level;
}
