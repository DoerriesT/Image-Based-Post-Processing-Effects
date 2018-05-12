#include "Utility.h"
#include <fstream>
#include <cassert>
#include <glad\glad.h>
#include <windows.h>
#include <iostream>
#include <sstream>
#include <cctype>
#include <al.h>
#include <chrono>
#include <ctime> 
#include <time.h>
#include <thread>
#include <direct.h>
#include <iomanip>

char* readTextResourceFile(const std::string &_filename)
{
	std::ifstream file(_filename, std::ios::ate);
	file.exceptions(std::ifstream::badbit);

	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file " + _filename + "!");
	}
	size_t fileSize = (size_t)file.tellg();
	char *buffer = new char[fileSize + 1];
	file.seekg(0);

	//file.read(buffer, fileSize);
	char c;
	size_t position = 0;
	while (file.get(c))
	{
		//if (c != '\r') {
		buffer[position] = c;
		++position;
		assert(position <= fileSize);
	}
	buffer[position] = '\0';

	file.close();

	return buffer;
}

std::streamsize loadBinaryFile(const char *_filename, char **_buffer)
{
	std::ifstream file(_filename, std::ios::binary | std::ios::ate);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	//std::vector<char> buffer(size);
	*_buffer = new char[(uint32_t)size];
	if (file.read(*_buffer, size))
	{
		return size;
	}
	return -1;
}

void glErrorCheck(const std::string &_message)
{
	switch (glGetError())
	{
	case (GL_INVALID_ENUM):
		std::cout << _message << std::endl;
		std::cout << "GL_INVALID_ENUM" << std::endl;
		break;
	case (GL_INVALID_VALUE):
		std::cout << _message << std::endl;
		std::cout << "GL_INVALID_VALUE" << std::endl;
		break;
	case (GL_INVALID_OPERATION):
		std::cout << _message << std::endl;
		std::cout << "GL_INVALID_OPERATION" << std::endl;
		break;
	case (GL_OUT_OF_MEMORY):
		std::cout << _message << std::endl;
		std::cout << "GL_OUT_OF_MEMORY" << std::endl;
		break;
	case (GL_INVALID_FRAMEBUFFER_OPERATION):
		std::cout << _message << std::endl;
		std::cout << "GL_INVALID_FRAMEBUFFER_OPERATION" << std::endl;
		break;
	}
}

void alErrorCheck(const std::string & _message)
{
	switch (alGetError())
	{
	case (AL_INVALID_NAME):
		std::cout << _message << std::endl;
		std::cout << "AL_INVALID_NAME" << std::endl;
		break;
	case (AL_INVALID_ENUM):
		std::cout << _message << std::endl;
		std::cout << "AL_INVALID_ENUM" << std::endl;
		break;
	case (AL_INVALID_VALUE):
		std::cout << _message << std::endl;
		std::cout << "AL_INVALID_VALUE" << std::endl;
		break;
	case (AL_INVALID_OPERATION):
		std::cout << _message << std::endl;
		std::cout << "AL_INVALID_OPERATION" << std::endl;
		break;
	case (AL_OUT_OF_MEMORY):
		std::cout << _message << std::endl;
		std::cout << "AL_OUT_OF_MEMORY" << std::endl;
		break;
	}
}

void createDummyMesh(GLuint &_VAO)
{
	// create buffers/arrays
	glGenVertexArrays(1, &_VAO);
	glBindVertexArray(_VAO);


	// vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * 4, (void*)0);

	glBindVertexArray(0);
}

static bool created = false;
static GLuint VAO;

void deleteDummyMesh()
{
	glBindVertexArray(VAO);
	glDisableVertexAttribArray(0);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VAO);
}

void bindDummyMesh()
{
	if (!created)
	{
		created = true;
		createDummyMesh(VAO);
	}

	assert(VAO);
	glBindVertexArray(VAO);
	glEnableVertexAttribArray(0);
}



glm::vec3 interpolateHermiteCurve(const double &_t, const glm::vec3 &_p0, const glm::vec3 &_p1, const glm::vec3 &_t0, const glm::vec3 &_t1)
{
	double t3 = _t * _t * _t;
	double t2 = _t * _t;
	double h1 = 2 * t3 - 3 * t2 + 1;
	double h2 = -2 * t3 + 3 * t2;
	double h3 = t3 - 2 * t2 + _t;
	double h4 = t3 - t2;

	return (float)h1 * _p0 + (float)h2 * _p1 + (float)h3 * _t0 + (float)h4 * _t1;
}

glm::quat nlerp(const glm::quat &_x, const glm::quat &_y, const float &_a)
{
	float cosom = _x.x * _y.x + _x.y * _y.y + _x.z * _y.z + _x.w * _y.w;
	float scale0 = 1.0f - _a;
	float scale1 = (cosom >= 0.0f) ? _a : -_a;
	glm::quat result;
	result = scale0 * _x + scale1 * _y;
	float s = (float)(1.0 / sqrt(result.x * result.x + result.y * result.y + result.z * result.z + result.w * result.w));
	result *= s;
	return result;
}

//base code from Fox32 (https://stackoverflow.com/a/5167641)
std::vector<std::string> Util::split(const std::string &str, const std::string &seperator)
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

bool Util::contains(const std::string &string, const std::string &contains)
{
	return string.find(contains) != std::string::npos;
}

void Util::ltrim(std::string &s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch)
	{
		return !std::isspace(ch);
	}));
}

void Util::rtrim(std::string &s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch)
	{
		return !std::isspace(ch);
	}).base(), s.end());
}

void Util::trim(std::string &str)
{
	ltrim(str);
	rtrim(str);
}


std::string Util::toLowerCase(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	return str;
}

bool Util::fileExists(const std::string &name)
{
	std::ifstream f(name.c_str());
	return f.good();
}

std::streampos Util::fileSize(const std::string &name)
{
	std::ifstream in(name, std::ifstream::ate | std::ifstream::binary);
	return in.tellg();
}

void Util::createDirectory(const std::string &path)
{
	_mkdir(path.c_str());
}

std::vector<std::string> Util::listDirectory(const std::string &directoryPath)
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
			if (!Util::equals(data.cFileName, ".") && !Util::equals(data.cFileName, ".."))
			{
				result.push_back(data.cFileName);
			}
		} while (FindNextFile(hFind, &data) != 0);
		FindClose(hFind);
	}
	return result;
}

bool Util::isDirectory(const std::string &directoryPath)
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

std::string Util::fileMD5Hash(const std::string & filePath)
{
	char *buf;
	auto size = loadBinaryFile(filePath.c_str(), &buf);
	MD5 md5;
	md5.update(buf, (unsigned int)size);
	md5.finalize();
	delete[] buf;
	return md5.hexdigest();
}

bool Util::readTextFile(const std::string & _filename, std::string & _result)
{
	char *buffer = nullptr;
	try
	{
		buffer = readTextResourceFile(_filename);
		_result = buffer;
	}
	catch (...)
	{
	}
	delete[] buffer;
	return buffer;
}

void Util::createFile(const std::string &filePath, const std::string &content)
{
	std::ofstream outfile(filePath, std::ios::binary);
	outfile << content;
	outfile.flush();
	outfile.close();
}

void Util::copyFile(const std::string &sourceFile, const std::string &destinationFile)
{
	std::ifstream  src(sourceFile, std::ios::binary);
	std::ofstream  dst(destinationFile, std::ios::binary);

	dst << src.rdbuf();
}

std::string Util::getPathFileName(const std::string &filePath)
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

std::string Util::getPathFileExtension(const std::string &filePath)
{
	return split(filePath, ".").back();
}

std::string Util::getPathLastPart(const std::string &filePath)
{
	return split(split(filePath, "/").back(), "\\").back();
}

size_t Util::grow(char *&buffer, size_t size)
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

void Util::sleep(unsigned int milliseconds)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

std::string Util::getFormatedTime()
{
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	struct tm buf;
	localtime_s(&buf, &in_time_t);

	std::stringstream ss;
	ss << std::put_time(&buf, "%Y-%m-%d-%H-%M-%S");
	return ss.str();
}

