#include "SponzaLevel.h"
#include "EntityComponentSystem\EntityManager.h"
#include <glm\ext.hpp>
#include <Graphics\Mesh.h>
#include <Graphics\EntityRenderData.h>
#include "EasingFunctions.h"
#include <Engine.h>
#include <Graphics\Material.h>
#include <Graphics\Texture.h>
#include <Graphics\Camera.h>
#include <random>

std::shared_ptr<Level> App::loadSponzaLevel()
{
	EntityManager &entityManager = EntityManager::getInstance();
	std::shared_ptr<Level> level = std::make_shared<Level>();

	// filepath
	level->m_filepath = "Resources/Levels/sponza/";

	// camera(s)
	std::shared_ptr<Camera> camera0 = std::make_shared<Camera>(glm::vec3(12.0f, 1.8f, 0.0f), glm::quat(glm::vec3(0.0f, glm::radians(-90.0f), 0.0f)));

	level->m_cameras.push_back(camera0);
	level->m_activeCameraIndex = 0;

	// exposure
	level->m_exposure = 1.0f;

	// water
	OceanParams oceanParams = { };
	oceanParams.m_enabled = false;
	oceanParams.m_level = 0.0f;
	oceanParams.m_normalizedWindDirection = glm::normalize(glm::vec2(0.8f, 0.6f));
	oceanParams.m_normalStrength = 0.1f;
	oceanParams.m_simulationResolution = 512;
	oceanParams.m_timeScale = 0.4f;
	oceanParams.m_waveAmplitude = 0.65f;
	oceanParams.m_waveChoppiness = 1.3f;
	oceanParams.m_waveSuppressionExponent = 6.0f;
	oceanParams.m_windSpeed = 600.0f;
	oceanParams.m_worldSize = 2000;
	level->m_oceanParams = oceanParams;

	// sun
	level->m_sun.m_direction = glm::normalize(glm::vec3(0.1f, 3.0f, -1.0f));

	/*- set skybox entity and environment maps*/
	const Entity *skyboxEntity = entityManager.createEntity();
	level->m_environment.m_skyboxEntity = skyboxEntity;

	//oceanLevel->environment.environmentMap = Texture::createTexture("Resources/Textures/oceanskybox.dds", true);

	// center
	const glm::vec3 centerProbePos = glm::vec3(0.0f, 2.0f, 0.0f);
	const AxisAlignedBoundingBox centerAabb = { glm::vec3(-9.5, -0.01, -2.4), glm::vec3(9.5, 13.0, 2.4) };

	// lower halls
	const AxisAlignedBoundingBox lowerHall0Aabb = { glm::vec3(-9.5, -0.01, 2.4), glm::vec3(9.5, 3.9, 6.1) };
	const glm::vec3 lowerHall0ProbePos = (lowerHall0Aabb.m_min + lowerHall0Aabb.m_max) * 0.5f;

	const AxisAlignedBoundingBox lowerHall1Aabb = { glm::vec3(-9.5, -0.01, -6.1), glm::vec3(9.5, 3.9, -2.4) };
	const glm::vec3 lowerHall1ProbePos = (lowerHall1Aabb.m_min + lowerHall1Aabb.m_max) * 0.5f;

	// lower end
	const AxisAlignedBoundingBox lowerEnd0Aabb = { glm::vec3(-13.7, -0.01, -6.1), glm::vec3(-9.5, 3.9, 6.1) };
	const glm::vec3 lowerEnd0ProbePos = (lowerEnd0Aabb.m_min + lowerEnd0Aabb.m_max) * 0.5f;

	const AxisAlignedBoundingBox lowerEnd1Aabb = { glm::vec3(9.5, -0.01, -6.1), glm::vec3( 13.65, 3.9, 6.1) };
	const glm::vec3 lowerEnd1ProbePos = (lowerEnd1Aabb.m_min + lowerEnd1Aabb.m_max) * 0.5f;

	// upper halls
	const AxisAlignedBoundingBox upperHall0Aabb = { glm::vec3(-9.8, 4.15, 2.8), glm::vec3(9.8, 8.7, 6.15) };
	const glm::vec3 upperHall0ProbePos = (upperHall0Aabb.m_min + upperHall0Aabb.m_max) * 0.5f;
	
	const AxisAlignedBoundingBox upperHall1Aabb = { glm::vec3(-9.8, 4.15, -6.1), glm::vec3(9.8, 8.7, -2.8) };
	const glm::vec3 upperHall1ProbePos = (upperHall1Aabb.m_min + upperHall1Aabb.m_max) * 0.5f;

	// upper end
	const AxisAlignedBoundingBox upperEnd0Aabb = { glm::vec3(-13.7, 4.15, -6.1), glm::vec3(-9.8, 8.7, 6.15) };
	const glm::vec3 upperEnd0ProbePos = (upperEnd0Aabb.m_min + upperEnd0Aabb.m_max) * 0.5f;

	const AxisAlignedBoundingBox upperEnd1Aabb = { glm::vec3(9.8, 4.15, -6.1), glm::vec3(13.65, 8.7, 6.15) };
	const glm::vec3 upperEnd1ProbePos = (upperEnd1Aabb.m_min + upperEnd1Aabb.m_max) * 0.5f;


	bool bakeReflections = SettingsManager::getInstance().getBoolSetting("graphics", "bake_reflections", false)->get();

	level->m_environment.m_environmentProbes.push_back(EnvironmentProbe::createEnvironmentProbe(centerProbePos, centerAabb, level->m_filepath + "centerProbe.dds", !bakeReflections));
	level->m_environment.m_environmentProbes.push_back(EnvironmentProbe::createEnvironmentProbe(lowerHall0ProbePos, lowerHall0Aabb, level->m_filepath + "lowerHall0Probe.dds", !bakeReflections));
	level->m_environment.m_environmentProbes.push_back(EnvironmentProbe::createEnvironmentProbe(lowerHall1ProbePos, lowerHall1Aabb, level->m_filepath + "lowerHall1Probe.dds", !bakeReflections));
	level->m_environment.m_environmentProbes.push_back(EnvironmentProbe::createEnvironmentProbe(lowerEnd0ProbePos, lowerEnd0Aabb, level->m_filepath + "lowerEnd0Probe.dds", !bakeReflections));
	level->m_environment.m_environmentProbes.push_back(EnvironmentProbe::createEnvironmentProbe(lowerEnd1ProbePos, lowerEnd1Aabb, level->m_filepath + "lowerEnd1Probe.dds", !bakeReflections));
	level->m_environment.m_environmentProbes.push_back(EnvironmentProbe::createEnvironmentProbe(upperHall0ProbePos, upperHall0Aabb, level->m_filepath + "upperHall0Probe.dds", !bakeReflections));
	level->m_environment.m_environmentProbes.push_back(EnvironmentProbe::createEnvironmentProbe(upperHall1ProbePos, upperHall1Aabb, level->m_filepath + "upperHall1Probe.dds", !bakeReflections));
	level->m_environment.m_environmentProbes.push_back(EnvironmentProbe::createEnvironmentProbe(upperEnd0ProbePos, upperEnd0Aabb, level->m_filepath + "upperEnd0Probe.dds", !bakeReflections));
	level->m_environment.m_environmentProbes.push_back(EnvironmentProbe::createEnvironmentProbe(upperEnd1ProbePos, upperEnd1Aabb, level->m_filepath + "upperEnd1Probe.dds", !bakeReflections));

	const glm::vec3 irradianceVolumeOrigin = glm::vec3(-14.0f, 1.0f, -7.0f);
	const glm::ivec3 irradianceVolumeDimensions = glm::ivec3(15, 6, 8);
	const float irradianceVolumeSpacing = 2.0f;

	bool bakeIrradianceVolume = SettingsManager::getInstance().getBoolSetting("graphics", "bake_irradiance_volume", false)->get();
	level->m_environment.m_irradianceVolume = bakeIrradianceVolume ? level->m_environment.m_irradianceVolume = IrradianceVolume::createIrradianceVolume(irradianceVolumeOrigin, irradianceVolumeDimensions, irradianceVolumeSpacing) :
		IrradianceVolume::createIrradianceVolume(irradianceVolumeOrigin, irradianceVolumeDimensions, irradianceVolumeSpacing, Texture::createTexture(level->m_filepath + "probes.dds", true));

	AtmosphereParams params;
	params.m_intensity = glm::vec3(1.0f, 1.0f, 1.0f);
	params.m_lightDir = level->m_sun.m_direction;
	params.m_mieBrightness = 100.0f;
	params.m_mieCollectionPower = 0.39f;
	params.m_mieDistribution = 63.0f;
	params.m_mieStrength = 264.0f;
	params.m_rayleighBrightness = 33.0f;
	params.m_rayleighCollectionPower = 0.81f;
	params.m_rayleighStrength = 139.0f;
	params.m_scatterStrength = 28.0f;
	params.m_spotBrightness = 1000.0f;
	params.m_stepCount = 16;
	params.m_surfaceHeight = 0.99f;

	level->m_environment.m_useAtmosphere = true;
	level->m_environment.m_isAtmosphereValid = false;
	level->m_environment.m_atmosphereParams = params;

	/*- set the lights in the scene*/
	level->m_lights.m_directionalLights.push_back(DirectionalLight::createDirectionalLight(Mobility::STATIC, params.m_intensity * 16.0f, level->m_sun.m_direction, true));
	float outerAngle = 50.0f;
	float innerAngle = 1.0f;
	float range = 25.0f;

	float lumens = 3400.0f;

	//level->lights.spotLights.push_back(SpotLight::createSpotLight(Mobility::DYNAMIC, lumens, glm::vec3(255, 211, 175) / 255.0f, glm::vec3(-2.5f + 6.0f, 0.7f, -0.8f), glm::vec3(-1.0f, -0.25f, -0.1f), outerAngle, innerAngle, range, true));
	//level->lights.spotLights.push_back(SpotLight::createSpotLight(Mobility::DYNAMIC, lumens, glm::vec3(255, 211, 175) / 255.0f, glm::vec3(-2.5f + 6.0f, 0.7f, 0.8f), glm::vec3(-1.0f, -0.25f, 0.1f), outerAngle, innerAngle, range, true));

	// objects
	{
		const Entity *sponzaEntity = entityManager.createEntity();
		level->m_entityMap["sponza"] = sponzaEntity;
		entityManager.addComponent<ModelComponent>(sponzaEntity, Model("Resources/Models/sponza.meshmat", true));
		entityManager.addComponent<TransformationComponent>(sponzaEntity, Mobility::STATIC);
		entityManager.addComponent<RenderableComponent>(sponzaEntity);
		//entityManager.addComponent<PhysicsComponent>(sponzaEntity, 0.0f, 1.0f, false);

		const Entity *carEntity = entityManager.createEntity();
		level->m_entityMap["car"] = carEntity;
		Model lamboModel("Resources/Models/aventador.meshmat", true);
		entityManager.addComponent<ModelComponent>(carEntity, lamboModel);
		entityManager.addComponent<TransformationComponent>(carEntity, Mobility::DYNAMIC, glm::vec3(6.0, 0.0, 0.0), glm::quat(glm::vec3(glm::radians(0.0f), 0.0f, 0.0f)), glm::vec3(1.0f));
		entityManager.addComponent<TransparencyComponent>(carEntity, lamboModel.getTransparentSubmeshes());
		entityManager.addComponent<RenderableComponent>(carEntity);
		//entityManager.addComponent<PhysicsComponent>(carEntity, 0.0f, 1.0f, false, true);

		std::vector<PathSegment> pathSegments;
		pathSegments.push_back(PathSegment(
			glm::vec3(-9.0, 0.0, 0.0),	// start pos
			glm::vec3(9.0, 0.0, 0.0),	// end pos
			glm::vec3(1.0f, 0.0f, 0.0f),						// start tangent
			glm::vec3(1.0f, 0.0f, 0.0f),						// end tangent
			1.0,														// duration
			linear));													// easing function
		pathSegments.push_back(PathSegment(
			glm::vec3(9.0, 0.0, 0.0),
			glm::vec3(-9.0, 0.0, 0.0),
			glm::vec3(-1.0f, 0.0f, 0.0),
			glm::vec3(-1.0f, 0.0f, 0.0),
			1.0,
			linear));
		//entityManager.addComponent<MovementPathComponent>(carEntity, pathSegments, Engine::getTime(), true);

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

		for (int i = 0; i < 10; ++i)
		{
			for (int j = 0; j < 2; ++j)
			{

				glm::vec3 color(1.0);
				glm::vec3 position = glm::vec3((i - 5.0f)  * 0.2f, 4.2f, j * 0.2f + 5.0f);

				const Entity *teapotEntity = entityManager.createEntity();
				level->m_entityMap["teapot" + std::to_string(i * 10 + j)] = teapotEntity;
				Model model("Resources/Models/teapot.meshmat", true);
				model[0].second.setRoughness(i / 10.0f);
				model[0].second.setMetallic(1.0f - j);
				entityManager.addComponent<ModelComponent>(teapotEntity, model);
				entityManager.addComponent<TransformationComponent>(teapotEntity, Mobility::DYNAMIC, position, glm::quat(glm::vec3(0.0, glm::radians(40.0f), 0.0f)), glm::vec3(1.0f));
				entityManager.addComponent<RenderableComponent>(teapotEntity);
			}
		}

		{
		const Entity *teapotEntity = entityManager.createEntity();
		level->m_entityMap["teapot_mb0"] = teapotEntity;
		Model model("Resources/Models/teapot.meshmat", true);
		model[0].second.setAlbedo(glm::vec4(1.0, 0.0, 0.0, 1.0));
		model[0].second.setRoughness(1.0f);
		model[0].second.setMetallic(0.0f);
		entityManager.addComponent<ModelComponent>(teapotEntity, model);
		auto *tc = entityManager.addComponent<TransformationComponent>(teapotEntity, Mobility::DYNAMIC, glm::vec3(0.25, 0.0, -0.5), glm::quat(glm::vec3(0.0, glm::radians(40.0f), 0.0f)), glm::vec3(2.0f));
		//entityManager.addComponent<RenderableComponent>(teapotEntity);

		tc->m_vel = glm::vec2(40.0, 0.0);
		}

		{
		const Entity *teapotEntity = entityManager.createEntity();
		level->m_entityMap["teapot_mb1"] = teapotEntity;
		Model model("Resources/Models/teapot.meshmat", true);
		model[0].second.setAlbedo(glm::vec4(0.0, 1.0, 0.0, 1.0));
		model[0].second.setRoughness(1.0f);
		model[0].second.setMetallic(0.0f);
		entityManager.addComponent<ModelComponent>(teapotEntity, model);
		auto *tc = entityManager.addComponent<TransformationComponent>(teapotEntity, Mobility::DYNAMIC, glm::vec3(), glm::quat(glm::vec3(0.0, glm::radians(40.0f), 0.0f)), glm::vec3(2.0f));
		//entityManager.addComponent<RenderableComponent>(teapotEntity);

		tc->m_vel = glm::vec2(0.0, 40.0);
		}
	}

	level->m_id = (size_t)1;
	level->m_valid = true;
	level->m_name = "sponza";
	level->m_loaded = true;

	return level;
}
