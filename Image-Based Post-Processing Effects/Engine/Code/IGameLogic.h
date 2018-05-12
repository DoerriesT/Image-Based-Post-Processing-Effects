#pragma once

class IGameLogic
{
public:
	virtual ~IGameLogic() = default;
	virtual void init() = 0;
	virtual void input(double currentTime, double timeDelta) = 0;
	virtual void update(double currentTime, double timeDelta) = 0;
	virtual void render() = 0;
};

