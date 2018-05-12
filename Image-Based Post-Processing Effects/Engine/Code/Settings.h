#pragma once
#include <string>
#include <cstdint>
#include <vector>
#include <functional>
#include <map>
#include <memory>
#include "Utilities\INIFile.h"

template<typename Type>
class Setting
{
public:
	Setting(const Type &_value, const std::string &_section, const std::string &_key, INIFile &_iniFile);
	Type get() const;
	void set(const Type &_value);
	void addListener(std::function<void(const Type &)> _listener);

private:
	Type value;
	std::string section;
	std::string key;
	INIFile &iniFile;
	std::vector<std::function<void(const Type &)>> listeners;
};

class SettingsManager
{
public:
	static SettingsManager &getInstance();

	SettingsManager(const SettingsManager &) = delete;
	SettingsManager(const SettingsManager &&) = delete;
	SettingsManager &operator= (const SettingsManager &) = delete;
	SettingsManager &operator= (const SettingsManager &&) = delete;
	std::shared_ptr<Setting<bool>> getBoolSetting(const std::string &_section, const std::string &_key, bool _defaultValue);
	std::shared_ptr<Setting<int>> getIntSetting(const std::string &_section, const std::string &_key, int _defaultValue);
	std::shared_ptr<Setting<double>> getDoubleSetting(const std::string &_section, const std::string &_key, double _defaultValue);
	std::shared_ptr<Setting<std::string>> getStringSetting(const std::string &_section, const std::string &_key, const std::string &_defaultValue);
	void saveToIni();

private:
	INIFile iniFile;
	std::map<std::pair<std::string, std::string>, std::shared_ptr<Setting<bool>>> boolSettingMap;
	std::map<std::pair<std::string, std::string>, std::shared_ptr<Setting<int>>> intSettingMap;
	std::map<std::pair<std::string, std::string>, std::shared_ptr<Setting<double>>> doubleSettingMap;
	std::map<std::pair<std::string, std::string>, std::shared_ptr<Setting<std::string>>> stringSettingMap;

	explicit SettingsManager(const std::string &_iniPath = "settings.ini");
	~SettingsManager() = default;
};

enum class WindowMode
{
	WINDOWED, BORDERLESS_FULLSCREEN, FULLSCREEN
};

template<typename Type>
inline Setting<Type>::Setting(const Type &_value, const std::string &_section, const std::string &_key, INIFile &_iniFile)
	:value(_value),
	section(_section),
	key(_key),
	iniFile(_iniFile)
{
}

template<typename Type>
inline Type Setting<Type>::get() const
{
	return value;
}

template<typename Type>
inline void Setting<Type>::set(const Type &_value)
{
	if (_value != value)
	{
		value = _value;
		//iniFile
		for (auto &listener : listeners)
		{
			listener(value);
		}
	}
}

template<>
inline void Setting<int>::set(const int &_value)
{
	if (_value != value)
	{
		value = _value;
		iniFile.setInt(section, key, value);
		for (auto &listener : listeners)
		{
			listener(value);
		}
	}
}

template<>
inline void Setting<bool>::set(const bool &_value)
{
	if (_value != value)
	{
		value = _value;
		iniFile.setBool(section, key, value);
		for (auto &listener : listeners)
		{
			listener(value);
		}
	}
}

template<>
inline void Setting<double>::set(const double &_value)
{
	if (_value != value)
	{
		value = _value;
		iniFile.setDouble(section, key, value);
		for (auto &listener : listeners)
		{
			listener(value);
		}
	}
}

template<>
inline void Setting<std::string>::set(const std::string &_value)
{
	if (_value != value)
	{
		value = _value;
		iniFile.setString(section, key, value);
		for (auto &listener : listeners)
		{
			listener(value);
		}
	}
}

template<typename Type>
inline void Setting<Type>::addListener(std::function<void(const Type &)> _listener)
{
	listeners.push_back(_listener);
}
