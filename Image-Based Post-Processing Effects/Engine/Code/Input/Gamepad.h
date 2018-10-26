#pragma once

struct Gamepad
{
	int m_id;
	float m_leftStickX;
	float m_leftStickY;
	float m_rightStickX;
	float m_rightStickY;
	float m_leftTrigger;
	float m_rightTrigger;
	bool m_buttonA;
	bool m_buttonB;
	bool m_buttonX;
	bool m_buttonY;
	bool m_dPadUp;
	bool m_dPadRight;
	bool m_dPadDown;
	bool m_dPadLeft;
	bool m_leftButton;
	bool m_rightButton;
	bool m_backButton;
	bool m_startButton;
	bool m_leftStick;
	bool m_rightStick;
};