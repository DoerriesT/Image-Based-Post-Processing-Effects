#pragma once
#include <glm\vec3.hpp>
#include <map>
#include <string>
#include "EntityComponentSystem\Component.h"
#include "Utilities\JSONUtility.h"
#include "Graphics\Camera.h"
#include "Graphics\CameraPath.h"
#include "Graphics\EnvironmentProbe.h"
#include "Graphics\Lights.h"

struct OceanParams
{
	bool m_enabled;
	float m_level;
	unsigned int m_simulationResolution;
	unsigned int m_worldSize;
	float m_waveAmplitude;
	float m_waveSuppressionExponent;
	float m_waveChoppiness;
	float m_windSpeed;
	float m_timeScale;
	float m_normalStrength;
	glm::vec2 m_normalizedWindDirection;
};

struct Sun
{
	glm::vec3 m_direction;
};

struct AtmosphereParams
{
	glm::vec3 m_lightDir;
	float m_surfaceHeight;
	float m_rayleighBrightness;
	float m_mieBrightness;
	float m_spotBrightness;
	float m_scatterStrength;
	float m_rayleighStrength;
	float m_mieStrength;
	float m_rayleighCollectionPower;
	float m_mieCollectionPower;
	float m_mieDistribution;
	glm::vec3 m_intensity;
	int m_stepCount;
};

struct Environment
{
	const Entity *m_skyboxEntity;
	std::shared_ptr<const Texture> m_environmentMap;
	std::vector<std::shared_ptr<EnvironmentProbe>> m_environmentProbes;
	std::shared_ptr<IrradianceVolume> m_irradianceVolume;
	bool m_useAtmosphere;
	bool m_isAtmosphereValid;
	AtmosphereParams m_atmosphereParams;
};

struct Level
{
	bool m_valid = false;
	bool m_loaded = false;
	size_t m_id;
	std::string m_name;
	std::string m_filepath;
	Lights m_lights;
	Sun m_sun;
	OceanParams m_oceanParams;
	float m_exposure;
	Environment m_environment;
	std::size_t m_activeCameraIndex;
	std::vector<std::shared_ptr<Camera>> m_cameras;
	std::map<std::string, const Entity *> m_entityMap;

	std::vector<CameraPath> m_cameraPaths;
};

void unloadLevel(Level &_level);