#include <algorithm>
#include "Settings.h"
#include "Utilities\Utility.h"

SettingsManager &SettingsManager::getInstance()
{
	static SettingsManager instance;
	return instance;
}

std::shared_ptr<Setting<bool>> SettingsManager::getBoolSetting(const std::string &_section, const std::string &_key, bool _defaultValue)
{
	auto pair = std::make_pair(_section, _key);
	if (contains(boolSettingMap, pair))
	{
		return boolSettingMap[pair];
	}
	else
	{
		bool value = iniFile.getBool(_section, _key, _defaultValue);
		std::shared_ptr<Setting<bool>> setting = std::make_shared<Setting<bool>>(value, _section, _key, iniFile);
		boolSettingMap[pair] = setting;
		return setting;
	}
}

std::shared_ptr<Setting<int>> SettingsManager::getIntSetting(const std::string &_section, const std::string &_key, int _defaultValue)
{
	auto pair = std::make_pair(_section, _key);
	if (contains(intSettingMap, pair))
	{
		return intSettingMap[pair];
	}
	else
	{
		int value = iniFile.getInt(_section, _key, _defaultValue);
		std::shared_ptr<Setting<int>> setting = std::make_shared<Setting<int>>(value, _section, _key, iniFile);
		intSettingMap[pair] = setting;
		return setting;
	}
}

std::shared_ptr<Setting<double>> SettingsManager::getDoubleSetting(const std::string &_section, const std::string &_key, double _defaultValue)
{
	auto pair = std::make_pair(_section, _key);
	if (contains(doubleSettingMap, pair))
	{
		return doubleSettingMap[pair];
	}
	else
	{
		double value = iniFile.getDouble(_section, _key, _defaultValue);
		std::shared_ptr<Setting<double>> setting = std::make_shared<Setting<double>>(value, _section, _key, iniFile);
		doubleSettingMap[pair] = setting;
		return setting;
	}
}

std::shared_ptr<Setting<std::string>> SettingsManager::getStringSetting(const std::string &_section, const std::string &_key, const std::string &_defaultValue)
{
	auto pair = std::make_pair(_section, _key);
	if (contains(stringSettingMap, pair))
	{
		return stringSettingMap[pair];
	}
	else
	{
		std::string value = iniFile.getString(_section, _key, _defaultValue);
		std::shared_ptr<Setting<std::string>> setting = std::make_shared<Setting<std::string>>(value, _section, _key, iniFile);
		stringSettingMap[pair] = setting;
		return setting;
	}
}

void SettingsManager::saveToIni()
{
	iniFile.save();
}

SettingsManager::SettingsManager(const std::string &_iniPath)
	:iniFile(_iniPath)
{
}
