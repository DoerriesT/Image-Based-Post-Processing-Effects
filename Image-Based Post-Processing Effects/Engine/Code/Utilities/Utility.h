#pragma once
#include <string>
#include <vector>
#include <iomanip>
#include "md5.h"

namespace Utility
{
	std::vector<char> readTextFile(const std::string &_filename);
	std::vector<char> readBinaryFile(const std::string &_filename);

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

	template< typename T >
	std::string to_hex(T i)
	{
		std::stringstream stream;
		stream << "0x"
			<< std::setfill('0') << std::setw(sizeof(T) * 2)
			<< std::hex << i;
		return stream.str();
	}
}