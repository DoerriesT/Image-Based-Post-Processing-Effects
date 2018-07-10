#pragma once

struct Gamepad
{
	int id;
	float leftStickX;
	float leftStickY;
	float rightStickX;
	float rightStickY;
	float leftTrigger;
	float rightTrigger;
	bool buttonA;
	bool buttonB;
	bool buttonX;
	bool buttonY;
	bool dPadUp;
	bool dPadRight;
	bool dPadDown;
	bool dPadLeft;
	bool leftButton;
	bool rightButton;
	bool backButton;
	bool startButton;
	bool leftStick;
	bool rightStick;
};