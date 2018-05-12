#include <algorithm>
#include "UserInput.h"

UserInput &UserInput::getInstance()
{
	static UserInput instance;
	return instance;
}


void UserInput::input()
{
	pressedChars.clear();

	mousePosDelta = (currentMousePos - previousMousePos);
	previousMousePos = currentMousePos;
}

glm::vec2 UserInput::getPreviousMousePos()
{
	return previousMousePos;
}

glm::vec2 UserInput::getCurrentMousePos()
{
	return currentMousePos;
}

glm::vec2 UserInput::getMousePosDelta()
{
	return mousePosDelta;
}

glm::vec2 UserInput::getScrollOffset()
{
	return scrollOffset;
}

const std::set<int> &UserInput::getPressedKeys()
{
	return pressedKeys;
}

const std::set<int> &UserInput::getPressedMouseButtons()
{
	return pressedMouseButtons;
}

bool UserInput::isMouseInsideWindow()
{
	return insideWindow;
}

bool UserInput::isKeyPressed(const int &_key)
{
	return pressedKeys.find(_key) != pressedKeys.end();
}

bool UserInput::isMouseButtonPressed(const int &_mouseButton)
{
	return pressedMouseButtons.find(_mouseButton) != pressedMouseButtons.end();
}

void UserInput::addKeyListener(IKeyListener *_listener)
{
	keyListeners.push_back(_listener);
}

void UserInput::removeKeyListener(IKeyListener *_listener)
{
	keyListeners.erase(std::remove(keyListeners.begin(), keyListeners.end(), _listener), keyListeners.end());
}

void UserInput::addCharListener(ICharListener *_listener)
{
	charListeners.push_back(_listener);
}

void UserInput::removeCharListener(ICharListener *_listener)
{
	charListeners.erase(std::remove(charListeners.begin(), charListeners.end(), _listener), charListeners.end());
}

void UserInput::addScrollListener(IScrollListener *_listener)
{
	scrollListeners.push_back(_listener);
}

void UserInput::removeScrollListener(IScrollListener *_listener)
{
	scrollListeners.erase(std::remove(scrollListeners.begin(), scrollListeners.end(), _listener), scrollListeners.end());
}

void UserInput::addMouseButtonListener(IMouseButtonListener *_listener)
{
	mouseButtonlisteners.push_back(_listener);
}

void UserInput::removeMouseButtonListener(IMouseButtonListener *_listener)
{
	mouseButtonlisteners.erase(std::remove(mouseButtonlisteners.begin(), mouseButtonlisteners.end(), _listener), mouseButtonlisteners.end());
}

void UserInput::onKey(const int &_key, const int &_action)
{
	for (IKeyListener *listener : keyListeners)
	{
		listener->onKey(_key, _action);
	}
	for (std::function<void(int, int)> callback : keyCallbacks)
	{
		callback(_key, _action);
	}

	if (_action == INPUT_RELEASE)
	{
		pressedKeys.erase(_key);
	}
	else if (_action == INPUT_PRESS)
	{
		pressedKeys.insert(_key);
	}
}

void UserInput::onChar(const int &_charKey)
{
	for (ICharListener *listener : charListeners)
	{
		listener->onChar(_charKey);
	}
	for (std::function<void(int)> callback : charCallbacks)
	{
		callback(_charKey);
	}
	pressedKeys.insert(_charKey);
}

void UserInput::onMouseButton(const int &_mouseButton, const int &_action)
{
	for (IMouseButtonListener *listener : mouseButtonlisteners)
	{
		listener->onMouseButton(_mouseButton, _action);
	}
	for (std::function<void(int, int)> callback : mouseButtonCallbacks)
	{
		callback(_mouseButton, _action);
	}
	if (_action == INPUT_RELEASE)
	{
		pressedMouseButtons.erase(_mouseButton);
	}
	else if (_action == INPUT_PRESS)
	{
		pressedMouseButtons.insert(_mouseButton);
	}
}

void UserInput::onMouseMove(const double &_x, const double &_y)
{
	currentMousePos.x = static_cast<float>(_x);
	currentMousePos.y = static_cast<float>(_y);
}

void UserInput::onMouseEnter(const bool &_entered)
{
	insideWindow = _entered;
}

void UserInput::onMouseScroll(const double &_xOffset, const double &_yOffset)
{
	for (IScrollListener *listener : scrollListeners)
	{
		listener->onScroll(_xOffset, _yOffset);
	}
	for (std::function<void(double, double)> callback : scrollCallbacks)
	{
		callback(_xOffset, _yOffset);
	}

	scrollOffset.x = static_cast<float>(_xOffset);
	scrollOffset.y = static_cast<float>(_yOffset);
}