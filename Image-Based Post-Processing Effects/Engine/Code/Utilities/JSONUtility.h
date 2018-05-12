#pragma once
#include <rapidjson\rapidjson.h>
#include <rapidjson\document.h>
#include <string>



namespace JSON 
{
	typedef rapidjson::GenericValue<rapidjson::UTF8<>> Value;
	typedef rapidjson::GenericObject<false, Value> Object;
	typedef rapidjson::GenericObject<true, Value> ConstObject;
	typedef rapidjson::GenericArray<false, Value> Array;
	typedef rapidjson::GenericArray<true, Value> ConstArray;

	Value* getMemberValue(const Object &jsonObject, const char *key);

	bool getString(const Object &jsonObject, const char *key, std::string &result);
	std::string getStringDefault(const Object &jsonObject, const char *key, const std::string &default);

	bool getFloat(const Object &jsonObject, const char *key, float &result);
	float getFloatDefault(const Object &jsonObject, const char *key, const float &default);

	bool getInt(const Object &jsonObject, const char *key, int &result);
	int getIntDefault(const Object &jsonObject, const char *key, const int &default);

	bool getBoolDefault(const Object &jsonObject, const char *key, const bool &default);
}