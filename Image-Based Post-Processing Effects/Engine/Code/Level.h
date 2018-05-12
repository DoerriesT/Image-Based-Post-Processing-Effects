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

struct SceneGraphNode
{
	const Entity *entity;
	std::vector<SceneGraphNode *> children;
};

struct Water
{
	bool enabled;
	float level;
	int simulatedSize;
	int worldSize;
	float tileSize;
	float waveAmplitude;
	float waveSuppressionExponent;
	float waveChoppiness;
	glm::vec2 normalizedWindDirection;
	float windSpeed;
};

struct Sun
{
	glm::vec3 direction;
};

struct AtmosphereParams
{
	glm::vec3 lightDir;
	float surfaceHeight;
	float rayleighBrightness;
	float mieBrightness;
	float spotBrightness;
	float scatterStrength;
	float rayleighStrength;
	float mieStrength;
	float rayleighCollectionPower;
	float mieCollectionPower;
	float mieDistribution;
	glm::vec3 intensity;
	int stepCount;
};

struct Environment
{
	const Entity *skyboxEntity;
	std::shared_ptr<const Texture> environmentMap;
	std::shared_ptr<EnvironmentProbe> environmentProbe;
	std::shared_ptr<const Texture> brdfMap;
	bool useAtmosphere;
	bool isAtmosphereValid;
	AtmosphereParams atmosphereParams;
};

struct Level
{
	bool valid = false;
	bool loaded = false;
	size_t id;
	std::string name;
	std::string filepath;
	Lights lights;
	Sun sun;
	Water water;
	float exposure;
	Environment environment;
	std::size_t activeCameraIndex;
	std::vector<std::shared_ptr<Camera>> cameras;
	std::map<std::string, const Entity *> entityMap;

	std::vector<CameraPath> cameraPaths;

	/*std::vector<std::shared_ptr<BaseComponent *>> parseComponents(const JSON::Array &_array);
	glm::vec3 parseVec3(const JSON::Array &_array);
	glm::vec4 parseVec4(const JSON::Array &_array);
	glm::quat parseQuat(const JSON::Array &_array);
	PathSegment parsePathSegment(const JSON::Object &_array);
	double (*(parseEasingFunction(const JSON::Object &_object)))(double, const double &);*/
};

void unloadLevel(Level &_level);