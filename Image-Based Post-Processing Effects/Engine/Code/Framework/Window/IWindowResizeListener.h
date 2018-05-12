#pragma once

class IWindowResizeListener
{
public:
	virtual ~IWindowResizeListener() = default;
	virtual void onResize(const unsigned int &width, const unsigned int &height) = 0;
};

