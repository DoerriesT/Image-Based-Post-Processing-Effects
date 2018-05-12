#pragma once
#include <map>
#include <iostream>

class INIFile
{
	friend std::ostream &operator<<(std::ostream &_os, const INIFile &_iniFile);

public:
	explicit INIFile(const std::string &_filename = "settings.ini");
	void load();
	void save() const;
	void print(std::ostream &_out) const;

	bool getStringChecked(const std::string &_section, const std::string &_key, std::string &_result);
	std::string getString(const std::string &_section, const std::string &_key, const std::string &_defaultValue);
	bool getBool(const std::string &_section, const std::string &_key, const bool &_defaultValue);
	int getInt(const std::string &_section, const std::string &_key, const int &_defaultValue);
	double getDouble(const std::string &_section, const std::string &_key, const double &_defaultValue);

	void setString(const std::string &_section, const std::string &_key, const std::string &_value);
	void setBool(const std::string &_section, const std::string &_key, const bool &_value);
	void setInt(const std::string &_section, const std::string &_key, const int &_value);
	void setDouble(const std::string &_section, const std::string &_key, const double &_value);

	

private:
	std::string filename;
	std::map<std::string, std::map<std::string, std::string>> dataMap;

	void parse(const char *_parseText);
	void put(const char *_section, const char *_key, const char *_value);
	void put(const std::string &_section, const std::string &_key, const std::string &_value);
};

