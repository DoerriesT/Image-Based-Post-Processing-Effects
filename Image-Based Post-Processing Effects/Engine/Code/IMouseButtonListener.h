#pragma once

class IMouseButtonListener
{
public:
	virtual void onMouseButton(const int &_mouseButton, const int &_action) = 0;
};