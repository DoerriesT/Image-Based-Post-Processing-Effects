#pragma once
#include <Level.h>

namespace App
{
	std::shared_ptr<Level> loadDefaultLevel();
	std::shared_ptr<Level> loadSponzaLevel();
	std::shared_ptr<Level> loadSibenikLevel();
}
