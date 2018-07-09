#pragma once
#include "InputTokens.h"

class IMouseButtonListener
{
public:
	virtual void onMouseButton(InputMouse _mouseButton, InputAction _action) = 0;
};