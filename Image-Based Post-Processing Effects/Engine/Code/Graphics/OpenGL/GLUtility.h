#pragma once
#include <string>
#include <vector>

namespace GLUtility
{
	void glErrorCheck(const std::string &_message = "");

	// returns true if all domain dimensions are evenly divisible by local work group size,
	// which means no wasted threads need to be started
	bool glDispatchComputeHelper(unsigned int _domainX, unsigned int _domainY, unsigned int _domainZ, unsigned int _localSizeX, unsigned int _localSizeY, unsigned int _localSizeZ);

	std::string shaderIncludeResolve(const std::string &_sourceCode);
	std::string shaderDefineInjection(const std::string &_sourceCode, const std::vector<std::pair<std::string, int>> &_defines);
}