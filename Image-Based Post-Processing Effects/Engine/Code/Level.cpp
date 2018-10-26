#include "Level.h"
#include "EntityComponentSystem\EntityManager.h"
//#include "Utilities\Utility.h"
//#include "Graphics\Material.h"
//#include "EasingFunctions.h"

//Level::Level(const std::string &_levelFile)
//{
//	const char *levelJSON = readTextResourceFile(_levelFile);
//
//	rapidjson::Document document;
//	document.Parse(levelJSON);
//
//	auto endMember = document.MemberEnd();
//	assert(document.IsObject());
//	JSON::Object jsonObject = document.GetObject();
//
//	// environment
//	{
//		bool result = true;
//
//		JSON::Object *jsonEnvironment = nullptr;
//		JSON::Value *value;
//		value = JSON::getMemberValue(jsonObject, "environment");
//		assert(value && value->IsObject());
//		*jsonEnvironment = value->GetObject();
//
//		// texture paths
//		std::string albedoPath;
//		result = JSON::getString(*jsonEnvironment, "albedo", albedoPath);
//		assert(result);
//		std::string irradiancePath;
//		result = JSON::getString(*jsonEnvironment, "irradiance", irradiancePath);
//		assert(result);
//		std::string radiancePath;
//		result = JSON::getString(*jsonEnvironment, "radiance", radiancePath);
//		assert(result);
//		std::string brdfPath;
//		result = JSON::getString(*jsonEnvironment, "brdf", brdfPath);
//		assert(result);
//
//		JSON::Value *componentsArray = nullptr;
//		componentsArray = JSON::getMemberValue(*jsonEnvironment, "components");
//		assert(componentsArray && componentsArray->IsArray());
//
//		std::vector<std::shared_ptr<BaseComponent *>> environmentComponents = parseComponents(componentsArray->GetArray());
//	}
//
//	// water
//	{
//		bool result = true;
//
//		JSON::Value *water = nullptr;
//		water = JSON::getMemberValue(jsonObject, "water");
//		assert(water && water->IsObject());
//		JSON::Object &object = water->GetObjectA();
//
//		bool enabled = false;
//		enabled = JSON::getBoolDefault(object, "enabled", false);
//
//		float level;
//		result = JSON::getFloat(object, "level", level);
//		assert(result);
//	}
//}
//
//std::vector<std::shared_ptr<BaseComponent *>> Level::parseComponents(const JSON::Array &_array)
//{
//	std::vector<std::shared_ptr<BaseComponent *>> components;
//	bool result = true;
//	for (auto &v : _array)
//	{
//		assert(v.IsObject());
//		JSON::Object &object = v.GetObjectA();
//		std::string type;
//		result = JSON::getString(object, "type", type);
//		assert(result);
//
//		if (type == "BoundingBoxComponent")
//		{
//			JSON::Value *maxCornerArray;
//			maxCornerArray = JSON::getMemberValue(object, "maxCorner");
//			assert(maxCornerArray && maxCornerArray->IsArray());
//
//			JSON::Value *rotationArray;
//			rotationArray = JSON::getMemberValue(object, "rotation");
//			assert(rotationArray && rotationArray->IsArray());
//
//			components.push_back(std::shared_ptr<BaseComponent *>(new BoundingBoxComponent(parseVec3(maxCornerArray->GetArray()), parseQuat(rotationArray->GetArray()))));
//		}
//		else if (type == "CleanupComponent")
//		{
//			assert(false);
//		}
//		else if (type == "CustomTransparencyShaderComponent")
//		{
//			assert(false);
//		}
//		else if (type == "CustomOpaqueShaderComponent")
//		{
//			assert(false);
//		}
//		else if (type == "GrabbedComponent")
//		{
//			assert(false);
//		}
//		else if (type == "ModelComponent")
//		{
//			std::string modelPath;
//			result = JSON::getString(object, "model", modelPath);
//			assert(result);
//			// TODO
//		}
//		else if (type == "MovementPathComponent")
//		{
//			std::vector<PathSegment> segments;
//			JSON::Value *pathSegments = nullptr;
//			pathSegments = JSON::getMemberValue(object, "pathSegments");
//			assert(pathSegments && pathSegments->IsArray());
//			for (auto &v : pathSegments->GetArray())
//			{
//				segments.push_back(parsePathSegment(v.GetObjectA()));
//			}
//
//			bool result = true;
//			bool repeat;
//			result = JSON::getBoolDefault(object, "repeat", false);
//			assert(result);
//
//			components.push_back(std::shared_ptr<BaseComponent *>(new MovementPathComponent(segments, 0, repeat)));
//
//		}
//		else if (type == "MovementComponent")
//		{
//			assert(false);
//		}
//		else if (type == "OutlineComponent")
//		{
//			bool result = true;
//			float scaleMultiplier;
//			result = JSON::getFloat(object, "scaleMultiplier", scaleMultiplier);
//			assert(result);
//
//			JSON::Value *outlineColor;
//			outlineColor = JSON::getMemberValue(object, "outlineColor");
//			assert(outlineColor && outlineColor->IsArray());
//
//			components.push_back(std::shared_ptr<BaseComponent *>(new OutlineComponent(scaleMultiplier, parseVec4(outlineColor->GetArray()))));
//		}
//		else if (type == "PerpetualRotationComponent")
//		{
//			JSON::Value *rotationIncrement;
//			rotationIncrement = JSON::getMemberValue(object, "rotationIncrement");
//			assert(rotationIncrement && rotationIncrement->IsArray());
//
//			components.push_back(std::shared_ptr<BaseComponent *>(new PerpetualRotationComponent(parseVec3(rotationIncrement->GetArray()))));
//		}
//		else if (type == "RenderableComponent")
//		{
//			components.push_back(std::shared_ptr<BaseComponent *>(new RenderableComponent()));
//		}
//		else if (type == "RotationComponent")
//		{
//			assert(false);
//		}
//		else if (type == "SoundComponent")
//		{
//			bool result = true;
//			std::string soundFile;
//			result = JSON::getString(object, "soundFile", soundFile);
//			assert(result);
//
//			std::string soundType;
//			result = JSON::getString(object, "soundType", soundType);
//			assert(result);
//
//			float volume;
//			result = JSON::getFloat(object, "volume", volume);
//			assert(result);
//
//			bool looping;
//			looping = JSON::getBoolDefault(object, "looping", false);
//
//			bool paused;
//			paused = JSON::getBoolDefault(object, "paused", false);
//
//			SoundType st;
//			if (soundType == "MUSIC")
//			{
//				st = SoundType::MUSIC;
//			}
//			else if (soundType == "EFFECT")
//			{
//				st = SoundType::EFFECT;
//			}
//			else
//			{
//				st = SoundType::UI;
//			}
//
//			components.push_back(std::shared_ptr<BaseComponent *>(new SoundComponent(soundFile, st, volume, looping, paused)));
//		}
//		else if (type == "TextureAtlasIndexComponent")
//		{
//			assert(false);
//		}
//		else if (type == "TransformationComponent")
//		{
//			JSON::Value *position;
//			position = JSON::getMemberValue(object, "position");
//			assert(position && position->IsArray());
//
//			JSON::Value *rotation;
//			rotation = JSON::getMemberValue(object, "rotation");
//			assert(rotation && rotation->IsArray());
//
//			float scale;
//			result = JSON::getFloat(object, "scale", scale);
//			assert(result);
//
//			components.push_back(std::shared_ptr<BaseComponent *>(new TransformationComponent(parseVec3(position->GetArray()), parseQuat(rotation->GetArray()), scale)));
//		}
//		else if (type == "TransparencyComponent")
//		{
//			components.push_back(std::shared_ptr<BaseComponent *>(new TransparencyComponent()));
//		}
//		else
//		{
//			assert(false);
//		}
//	}
//	return components;
//}
//
//glm::vec3 Level::parseVec3(const JSON::Array &_array)
//{
//	assert(_array.Size() == 3);
//	glm::vec3 vec;
//	for (int i = 0; i < 3; ++i)
//	{
//		auto &a = _array[i];
//		assert(a.IsFloat() || a.IsDouble()|| a.IsInt());
//		vec[i] = a.GetFloat();
//	}
//	return vec;
//}
//
//glm::vec4 Level::parseVec4(const JSON::Array & _array)
//{
//	assert(_array.Size() == 4);
//	glm::vec4 vec;
//	for (int i = 0; i < 4; ++i)
//	{
//		auto &a = _array[i];
//		assert(a.IsFloat() || a.IsDouble() || a.IsInt());
//		vec[i] = a.GetFloat();
//	}
//	return vec;
//}
//
//glm::quat Level::parseQuat(const JSON::Array &_array)
//{
//	assert(_array.Size() == 3);
//	glm::quat q;
//	for (int i = 0; i < 3; ++i)
//	{
//		auto &a = _array[i];
//		assert(a.IsFloat() || a.IsDouble() || a.IsInt());
//		q[i] = a.GetFloat();
//	}
//	return q;
//}
//
//PathSegment Level::parsePathSegment(const JSON::Object &_object)
//{
//	JSON::Value *startPosition;
//	startPosition = JSON::getMemberValue(_object, "startPosition");
//	assert(startPosition && startPosition->IsArray());
//
//	JSON::Value *endPosition;
//	endPosition = JSON::getMemberValue(_object, "endPosition");
//	assert(endPosition && endPosition->IsArray());
//
//	JSON::Value *startTangent;
//	startTangent = JSON::getMemberValue(_object, "startTangent");
//	assert(startTangent && startTangent->IsArray());
//	
//	JSON::Value *endTangent;
//	endTangent = JSON::getMemberValue(_object, "endTangent");
//	assert(endTangent && endTangent->IsArray());
//
//	bool result = true;
//
//	float totalDuration;
//	result = JSON::getFloat(_object, "totalDuration", totalDuration);
//	assert(result);
//
//	JSON::Value *easingFunction;
//	easingFunction = JSON::getMemberValue(_object, "easingFunction");
//	assert(easingFunction && startPosition->IsString());
//
//	return PathSegment(parseVec3(startPosition->GetArray()), parseVec3(endPosition->GetArray()), parseVec3(startTangent->GetArray()), parseVec3(endTangent->GetArray()), totalDuration, parseEasingFunction(easingFunction->GetObjectA()));
//}
//
//double(*(Level::parseEasingFunction(const JSON::Object &_object)))(double, const double &)
//{
//	std::string easingFunction;
//	bool result = JSON::getString(_object, "easingFunction", easingFunction);
//	assert(result);
//
//	double(*ef)(double, const double &) = nullptr;
//
//	if (easingFunction == "linear")
//	{
//		ef = linear;
//	}
//	else if (easingFunction == "quadraticEasingIn")
//	{
//		ef = quadraticEasingIn;
//	}
//	else if (easingFunction == "quadraticEasingOut")
//	{
//		ef = quadraticEasingOut;
//	}
//	else if (easingFunction == "quadraticEasingInOut")
//	{
//		ef = quadraticEasingInOut;
//	}
//	else if (easingFunction == "cubicEasingIn")
//	{
//		ef = cubicEasingIn;
//	}
//	else if (easingFunction == "cubicEasingOut")
//	{
//		ef = cubicEasingOut;
//	}
//	else if (easingFunction == "cubicEasingInOut")
//	{
//		ef = cubicEasingInOut;
//	}
//	else if (easingFunction == "quarticEasingIn")
//	{
//		ef = quarticEasingIn;
//	}
//	else if (easingFunction == "quarticEasingOut")
//	{
//		ef = quarticEasingOut;
//	}
//	else if (easingFunction == "quarticEasingInOut")
//	{
//		ef = quarticEasingInOut;
//	}
//	else if (easingFunction == "quinticEasingIn")
//	{
//		ef = quinticEasingIn;
//	}
//	else if (easingFunction == "quinticEasingOut")
//	{
//		ef = quinticEasingOut;
//	}
//	else if (easingFunction == "quinticEasingInOut")
//	{
//		ef = quinticEasingInOut;
//	}
//	else if (easingFunction == "sinusoidalEasingOut")
//	{
//		ef = sinusoidalEasingOut;
//	}
//	else if (easingFunction == "sinusoidalEasingInOut")
//	{
//		ef = sinusoidalEasingInOut;
//	}
//	else if (easingFunction == "exponentialEasingIn")
//	{
//		ef = exponentialEasingIn;
//	}
//	else if (easingFunction == "exponentialEasingOut")
//	{
//		ef = exponentialEasingOut;
//	}
//	else if (easingFunction == "exponentialEasingInOut")
//	{
//		ef = exponentialEasingInOut;
//	}
//	else if (easingFunction == "circularEasingIn")
//	{
//		ef = circularEasingIn;
//	}
//	else if (easingFunction == "circularEasingOut")
//	{
//		ef = circularEasingOut;
//	}
//	else if (easingFunction == "circularEasingInOut")
//	{
//		ef = circularEasingInOut;
//	}
//	else
//	{
//		assert(false);
//	}
//
//	return ef;
//}

void unloadLevel(Level &_level)
{
	if (!_level.m_valid)
	{
		return;
	}

	EntityManager &entityManager = EntityManager::getInstance();
	for (auto &v : _level.m_entityMap)
	{
		const Entity *entity = v.second;
		entityManager.destroyEntity(entity);
	}
	if (_level.m_environment.m_skyboxEntity)
	{
		entityManager.destroyEntity(_level.m_environment.m_skyboxEntity);
	}
	_level.m_environment.m_environmentMap.reset();
	_level.m_environment.m_environmentProbes.clear();
	for (auto &v : _level.m_lights.m_directionalLights)
	{
		v.reset();
	}
	for (auto &v : _level.m_lights.m_pointLights)
	{
		v.reset();
	}
	for (auto &v : _level.m_lights.m_spotLights)
	{
		v.reset();
	}

	_level.m_valid = false;
}