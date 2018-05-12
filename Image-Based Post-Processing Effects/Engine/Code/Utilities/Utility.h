#pragma once
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <glm\vec3.hpp>
#include <glm\gtc\quaternion.hpp>
#include <iomanip>
#include "md5.h"

char* readTextResourceFile(const std::string &_filename);

//Writes the contents of a file into a buffer, returns the read bytes or -1 if read failed. Buffer has to be deleted !
std::streamsize loadBinaryFile(const char *_filename, char **_buffer);

void glErrorCheck(const std::string &_message);

void alErrorCheck(const std::string &_message);

void bindDummyMesh();

namespace Util
{
	/* String manipulation */
	std::vector<std::string> split(const std::string &str, const std::string &seperator);

	inline bool equals(const char *a, const char *b)
	{
		return (strcmp(a, b) == 0);
	}

	inline bool equalsIgnoreCase(const char *a, const char *b)
	{
		return (_stricmp(a, b) == 0);
	}

	inline bool empty(const char* text)
	{
		return !text[0];
	}

	bool contains(const std::string &string, const std::string &contains);
	void ltrim(std::string &s);
	void rtrim(std::string &s);
	void trim(std::string &str);
	std::string toLowerCase(std::string str);

	// this method is not very fast
	std::string getPathFileName(const std::string &filePath);

	// this method is not very fast
	std::string getPathFileExtension(const std::string &filePath);

	// this method is not very fast
	std::string getPathLastPart(const std::string &filePath);

	/* File */

	bool fileExists(const std::string &name);
	std::streampos fileSize(const std::string &name);
	void createFile(const std::string &filePath, const std::string &content);
	void copyFile(const std::string &sourceFile, const std::string &destinationFile);

	void createDirectory(const std::string &path);
	std::vector<std::string> listDirectory(const std::string &directoryPath);
	bool isDirectory(const std::string &directoryPath);

	std::string fileMD5Hash(const std::string &filePath);


	bool readTextFile(const std::string &_filename, std::string &_result);

	/* Misc */
	// grows a char array by 50%, deletes the old array and returns the new size
	size_t grow(char *&buffer, size_t size);

	void sleep(unsigned int milliseconds);

	std::string getFormatedTime();
}

template<typename Base, typename T>
inline bool instanceof(const T *ptr)
{
	return dynamic_cast<const Base*>(ptr) != nullptr;
}

template<typename T>
inline bool find(const std::vector<T> &_vector, const T &_item, int &_position)
{
	auto position = std::find(_vector.cbegin(), _vector.cend(), _item);

	if (position < _vector.cend())
	{
		_position = (int)(position - _vector.cbegin());
		return true;
	}
	return false;
}

template<typename T>
inline bool contains(const std::vector<T> &_vector, const T &_item)
{
	return std::find(_vector.cbegin(), _vector.cend(), _item) != _vector.cend();
}

template<typename Key, typename Value>
inline bool contains(const std::map<Key, Value> &_map, const Key &_item)
{
	return _map.find(_item) != _map.cend();
}

template<typename Key, typename Value>
inline bool contains(const std::unordered_map<Key, Value> &_map, const Key &_item)
{
	return _map.find(_item) != _map.cend();
}

template<typename T>
inline bool contains(const std::set<T> &_set, const T &_item)
{
	return _set.find(_item) != _set.cend();
}

template<typename T>
inline bool contains(const std::unordered_set<T> &_set, const T &_item)
{
	return _set.find(_item) != _set.cend();
}

template<typename T>
inline void remove(std::vector<T> &_vector, const T &_item)
{
	_vector.erase(std::remove(_vector.begin(), _vector.end(), _item), _vector.end());
}

template<typename Key, typename Value>
inline void remove(std::map<Key, Value> &_map, const Key &_item)
{
	_map.erase(_item);
}

template<typename Key, typename Value>
inline void remove(std::unordered_map<Key, Value> &_map, const Key &_item)
{
	_map.erase(_item);
}

template<typename T>
inline void remove(std::set<T> &_set, const T &_item)
{
	_set.erase(_item);
}

template<typename T>
inline void remove(std::unordered_set<T> &_set, const T &_item)
{
	_set.erase(_item);
}

glm::vec3 interpolateHermiteCurve(double _t, const glm::vec3 &_p0, const glm::vec3 &_p1, const glm::vec3 &_t0, const glm::vec3 &_t1);

glm::quat nlerp(const glm::quat &_x, const glm::quat &_y, float _a);

template< typename T >
std::string to_hex(T i)
{
	std::stringstream stream;
	stream << "0x"
		<< std::setfill('0') << std::setw(sizeof(T)*2)
		<< std::hex << i;
	return stream.str();
}
