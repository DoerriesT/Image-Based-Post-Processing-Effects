#include "JSONUtility.h"

JSON::Value* JSON::getMemberValue(const Object &jsonObject, const char *key)
{
	auto member = jsonObject.FindMember(key);
	if (member != jsonObject.MemberEnd())
	{
		return &member->value;
	}
	return nullptr;
}

bool JSON::getString(const Object &jsonObject, const char *key, std::string &result)
{
	Value* value = getMemberValue(jsonObject, key);
	if (value && value->IsString())
	{
		result = value->GetString();
		return true;
	}
	return false;
}

std::string JSON::getStringDefault(const Object &jsonObject, const char *key, const std::string &default)
{
	std::string result;
	if (!getString(jsonObject, key, result))
	{
		return default;
	}
	return result;
}

bool JSON::getFloat(const Object &jsonObject, const char *key, float &result)
{
	Value* value = getMemberValue(jsonObject, key);
	if (value)
	{
		if (value->IsFloat())
		{
			result = value->GetFloat();
			return true;
		}
		if (value->IsInt())
		{
			result = (float)value->GetInt();
			return true;
		}
	}
	return false;
}

float JSON::getFloatDefault(const Object &jsonObject, const char *key, float default)
{
	float result;
	if (!getFloat(jsonObject, key, result))
	{
		return default;
	}
	return result;
}

bool JSON::getInt(const Object &jsonObject, const char *key, int &result)
{
	Value* value = getMemberValue(jsonObject, key);
	if (value && value->IsInt())
	{
		result = value->GetInt();
		return true;
	}
	return false;
}

int JSON::getIntDefault(const Object &jsonObject, const char *key, int default)
{
	int result;
	if (!getInt(jsonObject, key, result))
	{
		return default;
	}
	return result;
}

bool JSON::getBoolDefault(const Object &jsonObject, const char *key, bool default)
{
	Value* value = getMemberValue(jsonObject, key);
	if (value && value->IsBool())
	{
		return value->GetBool();
	}
	return default;
}
