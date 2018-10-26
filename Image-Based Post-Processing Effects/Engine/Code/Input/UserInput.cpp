#include <algorithm>
#include "UserInput.h"
#include "Gamepad.h"

UserInput &UserInput::getInstance()
{
	static UserInput instance;
	return instance;
}


void UserInput::input()
{
	m_pressedChars.clear();

	m_mousePosDelta = (m_currentMousePos - m_previousMousePos);
	m_previousMousePos = m_currentMousePos;
}

glm::vec2 UserInput::getPreviousMousePos()
{
	return m_previousMousePos;
}

glm::vec2 UserInput::getCurrentMousePos()
{
	return m_currentMousePos;
}

glm::vec2 UserInput::getMousePosDelta()
{
	return m_mousePosDelta;
}

glm::vec2 UserInput::getScrollOffset()
{
	return m_scrollOffset;
}

Gamepad UserInput::getGamepad()
{
	const std::vector<Gamepad> &gamepadVector = *m_gamepads;
	for (size_t i = 0; i < gamepadVector.size(); ++i)
	{
		if (gamepadVector[i].m_id != -1)
		{
			return gamepadVector[i];
		}
	}
	return { -1 };
}

Gamepad UserInput::getGamepad(int gamepadId)
{
	const std::vector<Gamepad> &gamepadVector = *m_gamepads;
	for (size_t i = 0; i < gamepadVector.size(); ++i)
	{
		if (gamepadVector[i].m_id == gamepadId)
		{
			return gamepadVector[i];
		}
	}
	return { -1 };
}

bool UserInput::isMouseInsideWindow()
{
	return m_insideWindow;
}

bool UserInput::isKeyPressed(InputKey _key)
{
	return m_pressedKeys.find(_key) != m_pressedKeys.end();
}

bool UserInput::isMouseButtonPressed(InputMouse _mouseButton)
{
	return m_pressedMouseButtons.find(_mouseButton) != m_pressedMouseButtons.end();
}

void UserInput::addKeyListener(IKeyListener *_listener)
{
	m_keyListeners.push_back(_listener);
}

void UserInput::removeKeyListener(IKeyListener *_listener)
{
	m_keyListeners.erase(std::remove(m_keyListeners.begin(), m_keyListeners.end(), _listener), m_keyListeners.end());
}

void UserInput::addCharListener(ICharListener *_listener)
{
	m_charListeners.push_back(_listener);
}

void UserInput::removeCharListener(ICharListener *_listener)
{
	m_charListeners.erase(std::remove(m_charListeners.begin(), m_charListeners.end(), _listener), m_charListeners.end());
}

void UserInput::addScrollListener(IScrollListener *_listener)
{
	m_scrollListeners.push_back(_listener);
}

void UserInput::removeScrollListener(IScrollListener *_listener)
{
	m_scrollListeners.erase(std::remove(m_scrollListeners.begin(), m_scrollListeners.end(), _listener), m_scrollListeners.end());
}

void UserInput::addMouseButtonListener(IMouseButtonListener *_listener)
{
	m_mouseButtonlisteners.push_back(_listener);
}

void UserInput::removeMouseButtonListener(IMouseButtonListener *_listener)
{
	m_mouseButtonlisteners.erase(std::remove(m_mouseButtonlisteners.begin(), m_mouseButtonlisteners.end(), _listener), m_mouseButtonlisteners.end());
}

void UserInput::onKey(int _key, int _action)
{
	InputKey key = static_cast<InputKey>(_key);
	InputAction action = static_cast<InputAction>(_action);

	for (IKeyListener *listener : m_keyListeners)
	{
		listener->onKey(key, action);
	}

	if (action == InputAction::RELEASE)
	{
		m_pressedKeys.erase(key);
	}
	else if (action == InputAction::PRESS)
	{
		m_pressedKeys.insert(key);
	}
}

void UserInput::onChar(int _charKey)
{
	InputKey charKey = static_cast<InputKey>(_charKey);

	for (ICharListener *listener : m_charListeners)
	{
		listener->onChar(charKey);
	}

	m_pressedKeys.insert(charKey);
}

void UserInput::onMouseButton(int _mouseButton, int _action)
{
	InputMouse mouseButton = static_cast<InputMouse>(_mouseButton);
	InputAction action = static_cast<InputAction>(_action);

	for (IMouseButtonListener *listener : m_mouseButtonlisteners)
	{
		listener->onMouseButton(mouseButton, action);
	}

	if (action == InputAction::RELEASE)
	{
		m_pressedMouseButtons.erase(mouseButton);
	}
	else if (action == InputAction::PRESS)
	{
		m_pressedMouseButtons.insert(mouseButton);
	}
}

void UserInput::onMouseMove(double _x, double _y)
{
	m_currentMousePos.x = static_cast<float>(_x);
	m_currentMousePos.y = static_cast<float>(_y);
}

void UserInput::onMouseEnter(bool _entered)
{
	m_insideWindow = _entered;
}

void UserInput::onMouseScroll(double _xOffset, double _yOffset)
{
	for (IScrollListener *listener : m_scrollListeners)
	{
		listener->onScroll(_xOffset, _yOffset);
	}

	m_scrollOffset.x = static_cast<float>(_xOffset);
	m_scrollOffset.y = static_cast<float>(_yOffset);
}

void UserInput::gamepadUpdate(const std::vector<Gamepad> *_gamepads)
{
	m_gamepads = _gamepads;
}
