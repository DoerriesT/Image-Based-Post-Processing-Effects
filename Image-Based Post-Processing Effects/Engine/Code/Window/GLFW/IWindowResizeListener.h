#pragma once

class IWindowResizeListener
{
public:
	virtual ~IWindowResizeListener() = default;
	virtual void onResize(unsigned int width, unsigned int height) = 0;
};

