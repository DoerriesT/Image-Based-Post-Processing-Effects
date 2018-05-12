#pragma once
#include <IGameLogic.h>

class Application : public IGameLogic
{
public:
	void init();
	void input(const double &currentTime, const double &timeDelta);
	void update(const double &currentTime, const double &timeDelta);
	void render();
};