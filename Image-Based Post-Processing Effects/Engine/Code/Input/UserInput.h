#pragma once
#include <vector>
#include <set>
#include <glm\vec2.hpp>
#include <functional>
#include "Window\GLFW\IInputListener.h"
#include "ICharListener.h"
#include "IKeyListener.h"
#include "IScrollListener.h"
#include "IMouseButtonListener.h"
#include "InputTokens.h"


class UserInput :public IInputListener
{
public:
	static UserInput &getInstance();

	UserInput(const UserInput &) = delete;
	UserInput(const UserInput &&) = delete;
	UserInput &operator= (const UserInput &) = delete;
	UserInput &operator= (const UserInput &&) = delete;
	void input();
	glm::vec2 getPreviousMousePos();
	glm::vec2 getCurrentMousePos();
	glm::vec2 getMousePosDelta();
	glm::vec2 getScrollOffset();
	Gamepad getGamepad();
	Gamepad getGamepad(int gamepadId);
	bool isMouseInsideWindow();
	bool isKeyPressed(InputKey key);
	bool isMouseButtonPressed(InputMouse mouseButton);
	void addKeyListener(IKeyListener *_listener);
	void removeKeyListener(IKeyListener *_listener);
	void addCharListener(ICharListener *_listener);
	void removeCharListener(ICharListener *_listener);
	void addScrollListener(IScrollListener *_listener);
	void removeScrollListener(IScrollListener *_listener);
	void addMouseButtonListener(IMouseButtonListener *_listener);
	void removeMouseButtonListener(IMouseButtonListener *_listener);
	void onKey(int _key, int _action) override;
	void onChar(int _charKey) override;
	void onMouseButton(int _mouseButton, int _action) override;
	void onMouseMove(double _x, double _y) override;
	void onMouseEnter(bool _entered) override;
	void onMouseScroll(double _xOffset, double _yOffset) override;
	void gamepadUpdate(const std::vector<Gamepad> *_gamepadData) override;

private:
	glm::vec2 m_previousMousePos;
	glm::vec2 m_currentMousePos;
	glm::vec2 m_mousePosDelta;
	glm::vec2 m_scrollOffset;
	bool m_insideWindow;
	std::vector<IKeyListener*> m_keyListeners;
	std::vector<ICharListener*> m_charListeners;
	std::vector<IScrollListener*> m_scrollListeners;
	std::vector<IMouseButtonListener*> m_mouseButtonlisteners;
	std::set<InputKey> m_pressedKeys;
	std::set<InputMouse> m_pressedMouseButtons;
	std::set<InputKey> m_pressedChars;
	const std::vector<Gamepad> *m_gamepads;

	UserInput() = default;
};