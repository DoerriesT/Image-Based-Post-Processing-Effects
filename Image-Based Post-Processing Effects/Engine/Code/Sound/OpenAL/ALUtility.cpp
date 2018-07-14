#include "ALUtility.h"
#include <al.h>
#include <iostream>

void ALUtility::alErrorCheck(const std::string & _message)
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