#include <algorithm>
#include "Settings.h"
#include "Utilities\ContainerUtility.h"

SettingsManager &SettingsManager::getInstance()
{
	static SettingsManager instance;
	return instance;
}

std::shared_ptr<Setting<bool>> SettingsManager::getBoolSetting(const std::string &_section, const std::string &_key, bool _defaultValue)
{
	auto pair = std::make_pair(_section, _key);
	if (ContainerUtility::contains(m_boolSettingMap, pair))
	{
		return m_boolSettingMap[pair];
	}
	else
	{
		bool value = m_iniFile.getBool(_section, _key, _defaultValue);
		std::shared_ptr<Setting<bool>> setting = std::make_shared<Setting<bool>>(value, _section, _key, m_iniFile);
		m_boolSettingMap[pair] = setting;
		return setting;
	}
}

std::shared_ptr<Setting<int>> SettingsManager::getIntSetting(const std::string &_section, const std::string &_key, int _defaultValue)
{
	auto pair = std::make_pair(_section, _key);
	if (ContainerUtility::contains(m_intSettingMap, pair))
	{
		return m_intSettingMap[pair];
	}
	else
	{
		int value = m_iniFile.getInt(_section, _key, _defaultValue);
		std::shared_ptr<Setting<int>> setting = std::make_shared<Setting<int>>(value, _section, _key, m_iniFile);
		m_intSettingMap[pair] = setting;
		return setting;
	}
}

std::shared_ptr<Setting<double>> SettingsManager::getDoubleSetting(const std::string &_section, const std::string &_key, double _defaultValue)
{
	auto pair = std::make_pair(_section, _key);
	if (ContainerUtility::contains(m_doubleSettingMap, pair))
	{
		return m_doubleSettingMap[pair];
	}
	else
	{
		double value = m_iniFile.getDouble(_section, _key, _defaultValue);
		std::shared_ptr<Setting<double>> setting = std::make_shared<Setting<double>>(value, _section, _key, m_iniFile);
		m_doubleSettingMap[pair] = setting;
		return setting;
	}
}

std::shared_ptr<Setting<std::string>> SettingsManager::getStringSetting(const std::string &_section, const std::string &_key, const std::string &_defaultValue)
{
	auto pair = std::make_pair(_section, _key);
	if (ContainerUtility::contains(m_stringSettingMap, pair))
	{
		return m_stringSettingMap[pair];
	}
	else
	{
		std::string value = m_iniFile.getString(_section, _key, _defaultValue);
		std::shared_ptr<Setting<std::string>> setting = std::make_shared<Setting<std::string>>(value, _section, _key, m_iniFile);
		m_stringSettingMap[pair] = setting;
		return setting;
	}
}

void SettingsManager::saveToIni()
{
	m_iniFile.save();
}

SettingsManager::SettingsManager(const std::string &_iniPath)
	:m_iniFile(_iniPath)
{
}
