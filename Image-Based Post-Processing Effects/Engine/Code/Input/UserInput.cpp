#include <algorithm>
#include "UserInput.h"
#define GLFW_INCLUDE_NONE
#include <GLFW\glfw3.h>

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

	for (int i = 0; i < 16; ++i)
	{
		connectedJoySticks[i] = glfwJoystickPresent(i);
		if (connectedJoySticks[i])
		{
			gamepadInputData[i].axisValues = glfwGetJoystickAxes(i, &gamepadInputData[i].axisCount);
			gamepadInputData[i].buttonValues = glfwGetJoystickButtons(i, &gamepadInputData[i].buttonCount);
		}
	}
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

const GamepadInputData &UserInput::getGamepadInputData(int gamepadId)
{
	return gamepadInputData[gamepadId];
}

bool UserInput::isMouseInsideWindow()
{
	return insideWindow;
}

bool UserInput::isKeyPressed(InputKey _key)
{
	return pressedKeys.find(_key) != pressedKeys.end();
}

bool UserInput::isMouseButtonPressed(InputMouse _mouseButton)
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

void UserInput::onKey(int _key, int _action)
{
	InputKey key = static_cast<InputKey>(_key);
	InputAction action = static_cast<InputAction>(_action);

	for (IKeyListener *listener : keyListeners)
	{
		listener->onKey(key, action);
	}

	if (action == InputAction::RELEASE)
	{
		pressedKeys.erase(key);
	}
	else if (action == InputAction::PRESS)
	{
		pressedKeys.insert(key);
	}
}

void UserInput::onChar(int _charKey)
{
	InputKey charKey = static_cast<InputKey>(_charKey);

	for (ICharListener *listener : charListeners)
	{
		listener->onChar(charKey);
	}

	pressedKeys.insert(charKey);
}

void UserInput::onMouseButton(int _mouseButton, int _action)
{
	InputMouse mouseButton = static_cast<InputMouse>(_mouseButton);
	InputAction action = static_cast<InputAction>(_action);

	for (IMouseButtonListener *listener : mouseButtonlisteners)
	{
		listener->onMouseButton(mouseButton, action);
	}

	if (action == InputAction::RELEASE)
	{
		pressedMouseButtons.erase(mouseButton);
	}
	else if (action == InputAction::PRESS)
	{
		pressedMouseButtons.insert(mouseButton);
	}
}

void UserInput::onMouseMove(double _x, double _y)
{
	currentMousePos.x = static_cast<float>(_x);
	currentMousePos.y = static_cast<float>(_y);
}

void UserInput::onMouseEnter(bool _entered)
{
	insideWindow = _entered;
}

void UserInput::onMouseScroll(double _xOffset, double _yOffset)
{
	for (IScrollListener *listener : scrollListeners)
	{
		listener->onScroll(_xOffset, _yOffset);
	}

	scrollOffset.x = static_cast<float>(_xOffset);
	scrollOffset.y = static_cast<float>(_yOffset);
}