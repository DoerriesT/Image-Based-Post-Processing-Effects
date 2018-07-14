#include "Utility.h"
#include <fstream>
#include <cassert>
#include <windows.h>
#include <iostream>
#include <sstream>
#include <cctype>
#include <chrono>
#include <ctime>
#include <thread>
#include <direct.h>
#include <iomanip>
#include <algorithm>

std::vector<char> Utility::readTextFile(const std::string & _filename)
{
	std::ifstream file(_filename, std::ios::ate);
	file.exceptions(std::ifstream::badbit);

	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file " + _filename + "!");
	}
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer;
	buffer.reserve(fileSize);
	file.seekg(0);

	//file.read(buffer, fileSize);
	char c;
	size_t position = 0;
	while (file.get(c))
	{
		//if (c != '\r') {
		buffer.push_back(c);
		++position;
		assert(position <= fileSize);
	}
	buffer.push_back('\0');

	file.close();

	return buffer;
}

std::vector<char> Utility::readBinaryFile(const std::string & _filename)
{
	std::ifstream file(_filename, std::ios::binary | std::ios::ate);

	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file " + _filename + "!");
	}

	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<char> buffer((unsigned int)size);
	if (file.read(buffer.data(), size))
	{
		return buffer;
	}
	else
	{
		throw std::runtime_error("failed to read file " + _filename + "!");
	}
}

//base code from Fox32 (https://stackoverflow.com/a/5167641)
std::vector<std::string> Utility::split(const std::string &str, const std::string &seperator)
{
	std::vector<std::string> output;
	std::string::size_type prev_pos = 0, pos = 0;
	while ((pos = str.find(seperator, pos)) != std::string::npos)
	{
		std::string substring(str.substr(prev_pos, pos - prev_pos));
		output.push_back(substring);
		prev_pos = pos += seperator.size();
	}
	output.push_back(str.substr(prev_pos, pos - prev_pos));
	return output;
}

bool Utility::contains(const std::string &string, const std::string &contains)
{
	return string.find(contains) != std::string::npos;
}

void Utility::ltrim(std::string &s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch)
	{
		return !std::isspace(ch);
	}));
}

void Utility::rtrim(std::string &s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch)
	{
		return !std::isspace(ch);
	}).base(), s.end());
}

void Utility::trim(std::string &str)
{
	ltrim(str);
	rtrim(str);
}


std::string Utility::toLowerCase(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	return str;
}

bool Utility::fileExists(const std::string &name)
{
	std::ifstream f(name.c_str());
	return f.good();
}

std::streampos Utility::fileSize(const std::string &name)
{
	std::ifstream in(name, std::ifstream::ate | std::ifstream::binary);
	return in.tellg();
}

void Utility::createDirectory(const std::string &path)
{
	_mkdir(path.c_str());
}

std::vector<std::string> Utility::listDirectory(const std::string &directoryPath)
{
	std::vector<std::string> result;
	std::string pattern(directoryPath);
	pattern.append("\\*");
	WIN32_FIND_DATA data;
	HANDLE hFind;
	if ((hFind = FindFirstFile(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!Utility::equals(data.cFileName, ".") && !Utility::equals(data.cFileName, ".."))
			{
				result.push_back(data.cFileName);
			}
		} while (FindNextFile(hFind, &data) != 0);
		FindClose(hFind);
	}
	return result;
}

bool Utility::isDirectory(const std::string &directoryPath)
{
	std::string pattern(directoryPath);
	pattern.append("\\*");
	WIN32_FIND_DATA data;
	HANDLE hFind;
	if ((hFind = FindFirstFile(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
		return true;
	}
	return false;
}

std::string Utility::fileMD5Hash(const std::string & filePath)
{
	std::vector<char> buffer = readBinaryFile(filePath);
	MD5 md5;
	md5.update(buffer.data(), (unsigned int)buffer.size());
	md5.finalize();
	return md5.hexdigest();
}

bool Utility::readTextFile(const std::string & _filename, std::string & _result)
{
	// wtf?
	/*char *buffer = nullptr;
	try
	{
		buffer = readTextResourceFile(_filename);
		_result = buffer;
	}
	catch (...)
	{
	}
	delete[] buffer;
	return buffer;*/
	return false;
}

void Utility::createFile(const std::string &filePath, const std::string &content)
{
	std::ofstream outfile(filePath, std::ios::binary);
	outfile << content;
	outfile.flush();
	outfile.close();
}

void Utility::copyFile(const std::string &sourceFile, const std::string &destinationFile)
{
	std::ifstream  src(sourceFile, std::ios::binary);
	std::ofstream  dst(destinationFile, std::ios::binary);

	dst << src.rdbuf();
}

std::string Utility::getPathFileName(const std::string &filePath)
{
	auto f = getPathLastPart(filePath);

	auto list = split(f, ".");

	std::string result = "";
	size_t size = list.size();

	if (size <= 2)
	{
		return list.front();
	}

	for (size_t i = 0; i < size - 1; ++i)
	{
		if (i >= 1)
		{
			result += ".";
		}
		result += list[i];
	}
	return result;
}

std::string Utility::getPathFileExtension(const std::string &filePath)
{
	return split(filePath, ".").back();
}

std::string Utility::getPathLastPart(const std::string &filePath)
{
	return split(split(filePath, "/").back(), "\\").back();
}

size_t Utility::grow(char *&buffer, size_t size)
{
	const size_t newSize = (size >> 1) + size;
	char *newBuffer = new char[newSize];

	for (size_t i = 0; i < size; ++i)
	{
		newBuffer[i] = buffer[i];
	}

	delete[] buffer;

	buffer = newBuffer;
	return newSize;
}

void Utility::sleep(unsigned int milliseconds)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

std::string Utility::getFormatedTime()
{
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	struct tm buf;
	localtime_s(&buf, &in_time_t);

	std::stringstream ss;
	ss << std::put_time(&buf, "%Y-%m-%d-%H-%M-%S");
	return ss.str();
}

