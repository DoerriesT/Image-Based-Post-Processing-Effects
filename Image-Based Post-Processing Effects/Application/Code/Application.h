#pragma once
#include <IGameLogic.h>

class Application : public IGameLogic
{
public:
	void init();
	void input(double currentTime, double timeDelta);
	void update(double currentTime, double timeDelta);
	void render();
};