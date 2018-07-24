#include "GLUtility.h"
#include <glad\glad.h>
#include <iostream>
#include <sstream>
#include "Utilities\Utility.h"
#include "Utilities\ContainerUtility.h"

void GLUtility::glErrorCheck(const std::string &_message)
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

bool GLUtility::glDispatchComputeHelper(unsigned int _domainX, unsigned int _domainY, unsigned int _domainZ, unsigned int _localSizeX, unsigned int _localSizeY, unsigned int _localSizeZ)
{
	unsigned int numGroupsX = _domainX / _localSizeX + ((_domainX % _localSizeX == 0) ? 0 : 1);
	unsigned int numGroupsY = _domainY / _localSizeY + ((_domainY % _localSizeY == 0) ? 0 : 1);
	unsigned int numGroupsZ = _domainZ / _localSizeZ + ((_domainZ % _localSizeZ == 0) ? 0 : 1);
	glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
	return (_domainX % _localSizeX == 0) && (_domainY % _localSizeY == 0) && (_domainZ % _localSizeZ == 0);
}

bool isInComment(size_t _position, const std::string &_text)
{
	bool multiLineComment = false;
	bool singleLineComment = false;
	std::string lastSymbol;
	for (size_t i = 0; i < _text.size(); ++i)
	{
		std::string lastPlusCurrent = lastSymbol + _text[i];
		if (lastPlusCurrent == "//")
		{
			singleLineComment = true;
		}
		else if (lastPlusCurrent == "/*")
		{
			multiLineComment = true;
		}
		else if (lastPlusCurrent == "*/")
		{
			multiLineComment = false;
		}
		else if (_text[i] == '\n')
		{
			singleLineComment = false;
		}

		if (i == _position)
		{
			return multiLineComment || singleLineComment;
		}

		lastSymbol = _text[i];
	}
	return false;
}

std::string GLUtility::shaderIncludeResolve(const std::string & _sourceCode)
{
	std::string result = _sourceCode;
	std::vector<std::string> includedFiles;

	const auto includeWordLength = std::string("#include").length();

	// search for #include
	size_t pos = result.find("#include");
	while (pos != std::string::npos)
	{
		if (!isInComment(pos, result))
		{
			// find include file name
			std::string includeFileName;
			bool foundStart = false;
			size_t start = 0;
			size_t end;

			for (size_t i = pos + includeWordLength + 1; i < result.size(); ++i)
			{
				if (result[i] == '"')
				{
					if (!foundStart)
					{
						foundStart = true;
						start = i;
					}
					else
					{
						end = i;
						includeFileName = result.substr(start + 1, end - 1 - start);
						break;
					}
				}
			}

			// load file and replace #include with file conent
			if (!includeFileName.empty() && !ContainerUtility::contains(includedFiles, includeFileName))
			{
				includedFiles.push_back(includeFileName);
				std::string fileContent = Utility::readTextFile("Resources/Shaders/Include/" + includeFileName).data();

				result.replace(pos, end - pos + 1, fileContent);
			}
		}
		pos = result.find("#include", pos + 1);
	}

	return result;
}
