#include "INIFile.h"
#include "Utility.h"
#include <cassert>
#include <iostream>
#include <fstream>
#include <algorithm>

enum PARSING_MODE
{
	PARSING_SECTION,
	PARSING_KEY,
	PARSING_VALUE,
	PARSING_COMMENT,
	PARSING_NOTHING,
	PARSING_GARBAGE
};


char *popParsingMemory(const char *_parsingMemory, size_t &_memoryPosition)
{
	char *result = new char[_memoryPosition + 1];
	memcpy(result, _parsingMemory, _memoryPosition);
	result[_memoryPosition] = '\0';
	_memoryPosition = 0;
	return result;
}

char charToLowerCase(char _c)
{
	if (_c >= 'A' && _c <= 'Z')
	{
		return _c + ('z' - 'Z');
	}
	return _c;
}

void transformStringToLowerCase(std::string &_str)
{
	std::transform(_str.begin(), _str.end(), _str.begin(), charToLowerCase);
}

void INIFile::parse(const char *_parseText)
{
	dataMap.clear();

	char cc; //currently char read
	size_t parsePos = 0;

	size_t memorySize = 64;
	char *parseMemory = new char[memorySize];
	size_t memoryPos = 0;
	PARSING_MODE parsingMode = PARSING_NOTHING;

	char *currentSection = nullptr;
	char *currentKey = nullptr;

	while (true)
	{
		if (memoryPos >= memorySize)
		{
			memorySize = Util::grow(parseMemory, memorySize);
		}


		cc = _parseText[parsePos++];
		if (cc == '\n' || cc == '\r' || cc == '\0')
		{
			if (parsingMode == PARSING_VALUE)
			{
				char *currentValue = popParsingMemory(parseMemory, memoryPos);
				if (currentSection && currentKey)
				{
					put(currentSection, currentKey, currentValue);
				}
				delete[] currentValue;
			}
			if (cc == '\0') //EOF
			{
				break;
			}
			//reset for new line
			parsingMode = PARSING_NOTHING;
			memoryPos = 0;
		}
		else
		{
			switch (parsingMode)
			{
				case PARSING_NOTHING:
					if (cc == ';')
					{
						parsingMode = PARSING_COMMENT;
					}
					else if (cc == '[')
					{
						parsingMode = PARSING_SECTION;
					}
					else if (cc == '\t' || cc == ' ')  //skip whitespaces
					{
					}
					else
					{
						parsingMode = PARSING_KEY;
						parseMemory[memoryPos++] = charToLowerCase(cc);
					}
					break;

				case PARSING_KEY:
					if (cc == '=')
					{
						if (memoryPos > 0)
						{
							delete[] currentKey;
							currentKey = popParsingMemory(parseMemory, memoryPos);
							parsingMode = PARSING_VALUE;
						}
						else
						{
							parsingMode = PARSING_GARBAGE;
						}
					}
					else
					{
						parseMemory[memoryPos++] = charToLowerCase(cc);
					}
					break;

				case PARSING_VALUE:
					parseMemory[memoryPos++] = cc;
					break;

				case PARSING_SECTION:
					if (cc == ']')
					{
						delete[] currentSection;
						currentSection = popParsingMemory(parseMemory, memoryPos);
					}
					else
					{
						parseMemory[memoryPos++] = charToLowerCase(cc);
					}
					break;

				default:
					break;
			}
		}
	}

	delete[] parseMemory;
}


INIFile::INIFile(const std::string &_filename)
	:filename(_filename)
{
	load();
}

void INIFile::load()
{
	char *fileContent = "";

	try
	{
		fileContent = readTextResourceFile(filename);
	}
	catch (std::exception&)
	{
	}

	parse(fileContent);

#ifdef _DEBUG
	printf("-- INIFile loaded Data: --\n");
	std::cout << *this;
	printf("----\n");
#endif 
}

void INIFile::save() const
{
	//TODO: improve save, don't delete comments and keep loading order
	std::ofstream file(filename);
	if (file.is_open())
	{
		file << *this;
		file.close();
	}
	else
	{
		std::cout << "ERROR: Couldn't open \"" << filename << "\" file." << std::endl;
	}
}

void INIFile::print(std::ostream &_out) const
{
	for (auto const &ent1 : dataMap)
	{
		_out << '[' << ent1.first << ']' << std::endl;
		for (auto const &ent2 : ent1.second)
		{
			_out << ent2.first << '=' << ent2.second << std::endl;
		}
	}
}

std::ostream &operator<<(std::ostream &_os, const INIFile &_iniFile)
{
	_iniFile.print(_os);
	return _os;
}

void INIFile::put(const char *_section, const char *_key, const char *_value)
{
	put(std::string(_section), std::string(_key), std::string(_value));
}

void INIFile::put(const std::string &_section, const std::string &_key, const std::string &_value)
{
	std::string key = _key;
	std::string section = _section;
	transformStringToLowerCase(key);
	transformStringToLowerCase(section);

	dataMap[section][key] = _value;
}

void INIFile::setString(const std::string &_section, const std::string &_key, const std::string &_value)
{
	put(_section, _key, _value);
}

bool INIFile::getStringChecked(const std::string &_section, const std::string &_key, std::string &_result)
{
	std::string key = _key;
	std::string section = _section;
	transformStringToLowerCase(key);
	transformStringToLowerCase(section);

	if (dataMap.find(section) != dataMap.end())
	{
		auto dataSection = dataMap[section];
		if (dataSection.find(key) != dataSection.end())
		{
			_result = dataSection[key];
			return true;
		}
	}
	return false;
}

std::string INIFile::getString(const std::string &_section, const std::string &_key, const std::string &_defaultValue)
{
	std::string value;
	if (getStringChecked(_section, _key, value))
	{
		return value;
	}
	put(_section, _key, _defaultValue);
	return _defaultValue;
}

bool INIFile::getBool(const std::string &_section, const std::string &_key, const bool &_defaultValue)
{
	std::string value;
	if (getStringChecked(_section, _key, value))
	{
		if (value == "1")
		{
			return true;
		}
		if (value == "0")
		{
			return false;
		}
	}
	setBool(_section, _key, _defaultValue);
	return _defaultValue;
}

int INIFile::getInt(const std::string &_section, const std::string &_key, const int &_defaultValue)
{
	std::string value;
	if (getStringChecked(_section, _key, value))
	{
		try
		{
			return std::stoi(value);
		}
		catch (const std::invalid_argument&)
		{
		}
	}
	setInt(_section, _key, _defaultValue);
	return _defaultValue;
}

double INIFile::getDouble(const std::string &_section, const std::string &_key, const double &_defaultValue)
{
	std::string value;
	if (getStringChecked(_section, _key, value))
	{
		try
		{
			return std::stod(value);
		}
		catch (const std::invalid_argument&)
		{
		}
	}
	setDouble(_section, _key, _defaultValue);
	return _defaultValue;
}


void INIFile::setBool(const std::string &_section, const std::string &_key, const bool &_value)
{
	setString(_section, _key, std::string(_value ? "1" : "0"));
}

void INIFile::setInt(const std::string &_section, const std::string &_key, const int &_value)
{
	setString(_section, _key, std::to_string(_value));
}

void INIFile::setDouble(const std::string &_section, const std::string &_key, const double &_value)
{
	setString(_section, _key, std::to_string(_value));
}

