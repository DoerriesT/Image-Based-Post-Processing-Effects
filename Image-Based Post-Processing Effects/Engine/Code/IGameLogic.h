#pragma once

class IGameLogic
{
public:
	virtual ~IGameLogic() = default;
	virtual void init() = 0;
	virtual void input(const double &currentTime, const double &timeDelta) = 0;
	virtual void update(const double &currentTime, const double &timeDelta) = 0;
	virtual void render() = 0;
};

