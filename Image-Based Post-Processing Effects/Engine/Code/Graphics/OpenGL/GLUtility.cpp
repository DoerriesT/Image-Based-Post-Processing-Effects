#include "GLUtility.h"
#include <glad\glad.h>
#include <iostream>

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
