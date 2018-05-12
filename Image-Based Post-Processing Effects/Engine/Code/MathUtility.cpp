#include "MathUtility.h"
#include <random>


int MathUtility::randomInt(int a, int b) {
	thread_local std::mt19937 eng{ std::random_device{}() };
	std::uniform_int_distribution<int> dist(a, b);
	return dist(eng);
}
