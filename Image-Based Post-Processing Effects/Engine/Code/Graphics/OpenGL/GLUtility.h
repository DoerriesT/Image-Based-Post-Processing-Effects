#pragma once
#include <string>

namespace GLUtility
{
	void glErrorCheck(const std::string &_message = "");

	// returns true if all domain dimensions are evenly divisible by local work group size,
	// which means no wasted threads need to be started
	bool glDispatchComputeHelper(unsigned int _domainX, unsigned int _domainY, unsigned int _domainZ, unsigned int _localSizeX, unsigned int _localSizeY, unsigned int _localSizeZ);
}