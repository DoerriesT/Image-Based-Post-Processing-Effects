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
	Type m_value;
	std::string m_section;
	std::string m_key;
	INIFile &m_iniFile;
	std::vector<std::function<void(const Type &)>> m_listeners;
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
	INIFile m_iniFile;
	std::map<std::pair<std::string, std::string>, std::shared_ptr<Setting<bool>>> m_boolSettingMap;
	std::map<std::pair<std::string, std::string>, std::shared_ptr<Setting<int>>> m_intSettingMap;
	std::map<std::pair<std::string, std::string>, std::shared_ptr<Setting<double>>> m_doubleSettingMap;
	std::map<std::pair<std::string, std::string>, std::shared_ptr<Setting<std::string>>> m_stringSettingMap;

	explicit SettingsManager(const std::string &_iniPath = "settings.ini");
	~SettingsManager() = default;
};

enum class WindowMode
{
	WINDOWED, BORDERLESS_FULLSCREEN, FULLSCREEN
};

template<typename Type>
inline Setting<Type>::Setting(const Type &_value, const std::string &_section, const std::string &_key, INIFile &_iniFile)
	:m_value(_value),
	m_section(_section),
	m_key(_key),
	m_iniFile(_iniFile)
{
}

template<typename Type>
inline Type Setting<Type>::get() const
{
	return m_value;
}

template<typename Type>
inline void Setting<Type>::set(const Type &_value)
{
	if (_value != m_value)
	{
		m_value = _value;
		//iniFile
		for (auto &listener : m_listeners)
		{
			listener(m_value);
		}
	}
}

template<>
inline void Setting<int>::set(const int &_value)
{
	if (_value != m_value)
	{
		m_value = _value;
		m_iniFile.setInt(m_section, m_key, m_value);
		for (auto &listener : m_listeners)
		{
			listener(m_value);
		}
	}
}

template<>
inline void Setting<bool>::set(const bool &_value)
{
	if (_value != m_value)
	{
		m_value = _value;
		m_iniFile.setBool(m_section, m_key, m_value);
		for (auto &listener : m_listeners)
		{
			listener(m_value);
		}
	}
}

template<>
inline void Setting<double>::set(const double &_value)
{
	if (_value != m_value)
	{
		m_value = _value;
		m_iniFile.setDouble(m_section, m_key, m_value);
		for (auto &listener : m_listeners)
		{
			listener(m_value);
		}
	}
}

template<>
inline void Setting<std::string>::set(const std::string &_value)
{
	if (_value != m_value)
	{
		m_value = _value;
		m_iniFile.setString(m_section, m_key, m_value);
		for (auto &listener : m_listeners)
		{
			listener(m_value);
		}
	}
}

template<typename Type>
inline void Setting<Type>::addListener(std::function<void(const Type &)> _listener)
{
	m_listeners.push_back(_listener);
}
