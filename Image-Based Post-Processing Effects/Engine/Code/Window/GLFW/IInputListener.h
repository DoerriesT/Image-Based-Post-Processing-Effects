#pragma once

struct Gamepad;

class IInputListener
{
public:
	/**
	* Callback that is invoked when a key (e.g.: CTRL, not a character key like 'f') is pressed or released.
	*
	* @param key
	*            The id of the key.
	* @param action
	*            The action performed on the key (pressed, released, etc..).
	*/
	virtual void onKey(int _key, int _action) = 0;

	/**
	* Callback that is invoked when a character key (e.g.: 'f') is pressed.
	*
	* @param charKey
	*            The id of the pressed character key.
	*/
	virtual void onChar(int _charKey) = 0;

	/**
	* Callback that is invoked when a mouse button is pressed or released.
	*
	* @param mouseButton
	*            The id of the mouse button.
	* @param action
	*            The action performed on the mouse button (pressed, released, etc...)
	*/
	virtual void onMouseButton(int _mouseButton, int _action) = 0;

	/**
	* Callback that is invoked when the mouse is moved.
	*
	* @param x
	*            The current x coordinate of the mouse.
	* @param y
	*            The current y coordinate of the mouse.
	*/
	virtual void onMouseMove(double _x, double _y) = 0;

	/**
	* Callback that is invoked when the mouse enters or exits the window.
	*
	* @param entered
	*            true if mouse entered window, false otherwise.
	*/
	virtual void onMouseEnter(bool _entered) = 0;

	/**
	* Callback that is invoked when the mouse scroll wheel is used.
	*
	* @param xOffset
	*            The scroll offset in x direction.
	* @param yOffset
	*            The scroll offset in y direction.
	*/
	virtual void onMouseScroll(double _xOffset, double _yOffset) = 0;

	virtual void gamepadUpdate(const std::vector<Gamepad> *_gamepads) = 0;
};