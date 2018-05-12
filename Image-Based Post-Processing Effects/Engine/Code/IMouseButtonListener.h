#pragma once

class IMouseButtonListener
{
public:
	virtual void onMouseButton(int _mouseButton, int _action) = 0;
};