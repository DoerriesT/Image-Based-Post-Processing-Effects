#include "Levels.h"
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

std::shared_ptr<Level> App::loadDefaultLevel()
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
		level->environment.environmentProbe = EnvironmentProbe::createEnvironmentProbe(glm::vec3(0.0f, 2.0f, 0.0f), Texture::createTexture(level->filepath + "reflectance.dds", true), Texture::createTexture(level->filepath + "irradiance.dds", true));
	}
	else
	{
		level->environment.environmentProbe = EnvironmentProbe::createEnvironmentProbe(glm::vec3(0.0f, 2.0f, 0.0f));
	}

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


	// objects
	{
		const Entity *planeEntity = entityManager.createEntity();
		level->entityMap["plane"] = planeEntity;
		entityManager.addComponent<ModelComponent>(planeEntity, Model("Resources/Models/plane.meshmat", true));
		entityManager.addComponent<TransformationComponent>(planeEntity, glm::vec3(), glm::quat(glm::vec3(glm::radians(0.0f), 0.0f, 0.0f)), glm::vec3(100.0f));
		entityManager.addComponent<RenderableComponent>(planeEntity);
		entityManager.addComponent<PhysicsComponent>(planeEntity, 0.0f, 1.0f, false);

		int c = 0;
		std::default_random_engine e;
		std::uniform_real_distribution<float> dist(0.0f, 1.0f);
		for (int i = 0; i < 10; ++i)
		{
			for (int j = 0; j < 10; ++j)
			{
				for (int k = 0; k < 10; ++k)
				{
					const Entity *sphereEntity = entityManager.createEntity();
					level->entityMap["sphere" + std::to_string(c++)] = sphereEntity;
					Model sphereModel("Resources/Models/sphere.meshmat", true);
					sphereModel[0].second.setAlbedo(glm::vec4(dist(e), dist(e), dist(e), 1.0));
					entityManager.addComponent<ModelComponent>(sphereEntity, sphereModel);
					entityManager.addComponent<TransformationComponent>(sphereEntity, glm::vec3(i - 5.0, 20.0 + k, j - 5.0) * 0.1f, glm::quat(glm::vec3(glm::radians(0.0f), 0.0f, 0.0f)), glm::vec3(0.2f));
					entityManager.addComponent<RenderableComponent>(sphereEntity);
					entityManager.addComponent<PhysicsComponent>(sphereEntity, 1.0f, 0.9f, true);
				}
			}
		}

		/*for (int i = 0; i < 10; ++i)
		{
			for (int j = 0; j < 10; ++j)
			{

				glm::vec3 color(1.0);
				glm::vec3 position = glm::vec3(i - 5.0f, 0.0f, j - 5.0f) * 0.2;
				glm::vec3 bouncePos = position + glm::vec3(0.0f, 10.0f, 0.0f);

				const Entity *teapotEntity = entityManager.createEntity();
				level->entityMap["teapot" + std::to_string(i * 10 + j)] = teapotEntity;
				Model model("Resources/Models/teapot.meshmat", true);
				model[0].second.setRoughness(i / 10.0f);
				model[0].second.setMetallic(j / 10.0f);
				entityManager.addComponent<ModelComponent>(teapotEntity, model);
				entityManager.addComponent<TransformationComponent>(teapotEntity, position, glm::quat(glm::vec3(0.0, glm::radians(40.0f), 0.0f)), glm::vec3(1.0f));
				entityManager.addComponent<RenderableComponent>(teapotEntity);
			}
		}*/

		/*{
			const Entity *teapotEntity = entityManager.createEntity();
			level->entityMap["teapot"] = teapotEntity;
			Model model("Resources/Models/teapot.meshmat", true);
			model[0].second.setAlbedo(glm::vec4(1.0, 0.0, 0.0, 1.0));
			model[0].second.setRoughness(1.0f);
			model[0].second.setMetallic(0.0f);
			entityManager.addComponent<ModelComponent>(teapotEntity, model);
			auto *tc = entityManager.addComponent<TransformationComponent>(teapotEntity, glm::vec3(1.0, 0.0, -5.0), glm::quat(glm::vec3(0.0, glm::radians(40.0f), 0.0f)), glm::vec3(1.0f));
			entityManager.addComponent<RenderableComponent>(teapotEntity);

			tc->vel = glm::vec2(40.0, 0.0);
		}

		{
			const Entity *teapotEntity = entityManager.createEntity();
			level->entityMap["teapot"] = teapotEntity;
			Model model("Resources/Models/teapot.meshmat", true);
			model[0].second.setAlbedo(glm::vec4(0.0, 1.0, 0.0, 1.0));
			model[0].second.setRoughness(1.0f);
			model[0].second.setMetallic(0.0f);
			entityManager.addComponent<ModelComponent>(teapotEntity, model);
			auto *tc = entityManager.addComponent<TransformationComponent>(teapotEntity, glm::vec3(), glm::quat(glm::vec3(0.0, glm::radians(40.0f), 0.0f)), glm::vec3(1.0f));
			entityManager.addComponent<RenderableComponent>(teapotEntity);

			tc->vel = glm::vec2(0.0, 40.0);
		}*/
	}

	level->id = (size_t)0;
	level->valid = true;
	level->name = "default";
	level->loaded = true;

	return level;
}

std::shared_ptr<Level> App::loadSponzaLevel()
{
	EntityManager &entityManager = EntityManager::getInstance();
	std::shared_ptr<Level> level = std::make_shared<Level>();

	// filepath
	level->filepath = "Resources/Levels/sponza/";

	// camera(s)
	std::shared_ptr<Camera> camera0 = std::make_shared<Camera>(glm::vec3(0.0f, 1.5f, 2.0f), glm::quat(glm::vec3(glm::radians(0.0f), 0.0f, 0.0f)));

	level->cameras.push_back(camera0);
	level->activeCameraIndex = 0;

	// exposure
	level->exposure = 1.0f;

	// water
	level->water.enabled = false;
	level->water.level = 1.0;

	// sun
	level->sun.direction = glm::normalize(glm::vec3(0.0f, 2.0f, -1.0f));

	/*- set skybox entity and environment maps*/
	const Entity *skyboxEntity = entityManager.createEntity();
	level->environment.skyboxEntity = skyboxEntity;

	//oceanLevel->environment.environmentMap = Texture::createTexture("Resources/Textures/oceanskybox.dds", true);
	if (SettingsManager::getInstance().getBoolSetting("graphics", "load_environment_from_file", true)->get())
	{
		level->environment.environmentProbe = EnvironmentProbe::createEnvironmentProbe(glm::vec3(0.0f, 2.0f, 0.0f), Texture::createTexture(level->filepath + "reflectance.dds", true), Texture::createTexture(level->filepath + "irradiance.dds", true));
	}
	else
	{
		level->environment.environmentProbe = EnvironmentProbe::createEnvironmentProbe(glm::vec3(0.0f, 2.0f, 0.0f));
	}

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
	level->lights.directionalLights.push_back(DirectionalLight::createDirectionalLight(params.intensity * 32.0f, level->sun.direction, true));


	// objects
	{
		const Entity *sponzaEntity = entityManager.createEntity();
		level->entityMap["sponza"] = sponzaEntity;
		entityManager.addComponent<ModelComponent>(sponzaEntity, Model("Resources/Models/sponza.meshmat", true));
		entityManager.addComponent<TransformationComponent>(sponzaEntity);
		entityManager.addComponent<RenderableComponent>(sponzaEntity);
		//entityManager.addComponent<PhysicsComponent>(sponzaEntity, 0.0f, 1.0f, false);

		const Entity *carEntity = entityManager.createEntity();
		level->entityMap["car"] = carEntity;
		Model lamboModel("Resources/Models/aventador.meshmat", true);
		entityManager.addComponent<ModelComponent>(carEntity, lamboModel);
		entityManager.addComponent<TransformationComponent>(carEntity, glm::vec3(0.0, 0.0, 0.0), glm::quat(glm::vec3(glm::radians(0.0f), 0.0f, 0.0f)), glm::vec3(1.0f));
		entityManager.addComponent<TransparencyComponent>(carEntity, lamboModel.getTransparentSubmeshes());
		entityManager.addComponent<RenderableComponent>(carEntity);
		//entityManager.addComponent<PhysicsComponent>(carEntity, 0.0f, 1.0f, false, true);

		std::vector<PathSegment> pathSegments;
		pathSegments.push_back(PathSegment(
			glm::vec3(-9.0, 0.0, 0.0),	// start pos
			glm::vec3(9.0, 0.0, 0.0),	// end pos
			glm::vec3(0.0f, 1.0f, 0.0f),						// start tangent
			glm::vec3(0.0f, 1.0f, 0.0f),						// end tangent
			5.0,														// duration
			cubicEasingInOut));													// easing function
		pathSegments.push_back(PathSegment(
			glm::vec3(9.0, 0.0, 0.0),
			glm::vec3(-9.0, 0.0, 0.0),
			glm::vec3(0.0f, -1.0f, 0.0),
			glm::vec3(0.0f, -1.0f, 0.0),
			5.0,
			cubicEasingInOut));
		//entityManager.addComponent<MovementPathComponent>(carEntity, pathSegments, Engine::getCurrentTime(), true);

		/*int c = 0;
		std::default_random_engine e;
		std::uniform_real_distribution<float> dist(0.0f, 1.0f);
		for (int i = 0; i < 10; ++i)
		{
			for (int j = 0; j < 50; ++j)
			{
					const Entity *sphereEntity = entityManager.createEntity();
					level->entityMap["sphere" + std::to_string(c++)] = sphereEntity;
					Model sphereModel("Resources/Models/sphere.meshmat", true);
					sphereModel[0].second.setAlbedo(glm::vec4(dist(e), dist(e), dist(e), 1.0));
					sphereModel[0].second.setRoughness(0.1f);
					sphereModel[0].second.setMetallic(1.0f);
					entityManager.addComponent<ModelComponent>(sphereEntity, sphereModel);
					entityManager.addComponent<TransformationComponent>(sphereEntity, glm::vec3(i* 0.5f, 15.0 + j * 0.5, 0.0), glm::quat(glm::vec3(glm::radians(0.0f), 0.0f, 0.0f)), glm::vec3(0.4f));
					entityManager.addComponent<RenderableComponent>(sphereEntity);
					entityManager.addComponent<PhysicsComponent>(sphereEntity, 1.f, 0.8f, true, false, true);
			}
		}*/

		/*for (int i = 0; i < 10; ++i)
		{
		for (int j = 0; j < 10; ++j)
		{

		glm::vec3 color(1.0);
		glm::vec3 position = glm::vec3(i - 5.0f, 0.0f, j - 5.0f) * 0.2;
		glm::vec3 bouncePos = position + glm::vec3(0.0f, 10.0f, 0.0f);

		const Entity *teapotEntity = entityManager.createEntity();
		level->entityMap["teapot" + std::to_string(i * 10 + j)] = teapotEntity;
		Model model("Resources/Models/teapot.meshmat", true);
		model[0].second.setRoughness(i / 10.0f);
		model[0].second.setMetallic(j / 10.0f);
		entityManager.addComponent<ModelComponent>(teapotEntity, model);
		entityManager.addComponent<TransformationComponent>(teapotEntity, position, glm::quat(glm::vec3(0.0, glm::radians(40.0f), 0.0f)), glm::vec3(1.0f));
		entityManager.addComponent<RenderableComponent>(teapotEntity);
		}
		}*/

		/*{
		const Entity *teapotEntity = entityManager.createEntity();
		level->entityMap["teapot"] = teapotEntity;
		Model model("Resources/Models/teapot.meshmat", true);
		model[0].second.setAlbedo(glm::vec4(1.0, 0.0, 0.0, 1.0));
		model[0].second.setRoughness(1.0f);
		model[0].second.setMetallic(0.0f);
		entityManager.addComponent<ModelComponent>(teapotEntity, model);
		auto *tc = entityManager.addComponent<TransformationComponent>(teapotEntity, glm::vec3(1.0, 0.0, -5.0), glm::quat(glm::vec3(0.0, glm::radians(40.0f), 0.0f)), glm::vec3(1.0f));
		entityManager.addComponent<RenderableComponent>(teapotEntity);

		tc->vel = glm::vec2(40.0, 0.0);
		}

		{
		const Entity *teapotEntity = entityManager.createEntity();
		level->entityMap["teapot"] = teapotEntity;
		Model model("Resources/Models/teapot.meshmat", true);
		model[0].second.setAlbedo(glm::vec4(0.0, 1.0, 0.0, 1.0));
		model[0].second.setRoughness(1.0f);
		model[0].second.setMetallic(0.0f);
		entityManager.addComponent<ModelComponent>(teapotEntity, model);
		auto *tc = entityManager.addComponent<TransformationComponent>(teapotEntity, glm::vec3(), glm::quat(glm::vec3(0.0, glm::radians(40.0f), 0.0f)), glm::vec3(1.0f));
		entityManager.addComponent<RenderableComponent>(teapotEntity);

		tc->vel = glm::vec2(0.0, 40.0);
		}*/
	}

	level->id = (size_t)1;
	level->valid = true;
	level->name = "sponza";
	level->loaded = true;

	return level;
}
