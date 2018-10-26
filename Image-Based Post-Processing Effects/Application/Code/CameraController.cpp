#include "CameraController.h"
#include <cassert>
#include <Input\UserInput.h>
#include <Graphics\Camera.h>
#include <iostream>
#include <Input\Gamepad.h>
#include <Engine.h>
#include <Window/Window.h>

namespace App
{
	CameraController::CameraController(std::shared_ptr<Camera> _camera)
		:camera(_camera),
		userInput(UserInput::getInstance()),
		grabbedMouse(false),
		smoothFactor(0.0f)
	{
	}
	void CameraController::setCamera(std::shared_ptr<Camera> _camera)
	{
		camera = _camera;
	}

	void CameraController::input(double _currentTime, double _timeDelta)
	{
		assert(camera);
		Gamepad gamepad = userInput.getGamepad();

		glm::vec3 cameraTranslation;

		if (gamepad.m_id != -1)
		{
			camera->rotate(glm::vec3(-gamepad.m_rightStickY, gamepad.m_rightStickX, 0.0) * (float)_timeDelta * 2.0f);
			cameraTranslation.z = -5.0f * gamepad.m_leftStickY * (float)_timeDelta;
			cameraTranslation.x = 5.0f * gamepad.m_leftStickX * (float)_timeDelta;
			camera->translate(cameraTranslation);
		}

		bool pressed = false;
		float mod = 1.0f;

		glm::vec2 mouseDelta = {};

		if (userInput.isMouseButtonPressed(InputMouse::BUTTON_RIGHT))
		{
			if (!grabbedMouse)
			{
				//mouseHistory = glm::vec2(0.0f);
				grabbedMouse = true;
				Engine::getInstance()->getWindow()->grabMouse(grabbedMouse);
			}
			mouseDelta = userInput.getMousePosDelta();
		}
		else
		{
			if (grabbedMouse)
			{
				grabbedMouse = false;
				Engine::getInstance()->getWindow()->grabMouse(grabbedMouse);
			}
		}

		mouseHistory = glm::mix(mouseDelta, mouseHistory, smoothFactor);
		if (glm::dot(mouseHistory, mouseHistory) > 0.0f)
		{
			camera->rotate(glm::vec3(mouseHistory.y * 0.005f, mouseHistory.x * 0.005f, 0.0));
		}
		

		if (userInput.isKeyPressed(InputKey::UP))
		{
			camera->rotate(glm::vec3(-_timeDelta, 0.0f, 0.0));
		}
		if (userInput.isKeyPressed(InputKey::DOWN))
		{
			camera->rotate(glm::vec3(_timeDelta, 0.0f, 0.0));
		}
		if (userInput.isKeyPressed(InputKey::LEFT))
		{
			camera->rotate(glm::vec3(0.0f, -_timeDelta, 0.0));
		}
		if (userInput.isKeyPressed(InputKey::RIGHT))
		{
			camera->rotate(glm::vec3(0.0f, _timeDelta, 0.0));
		}

		if (userInput.isKeyPressed(InputKey::LEFT_SHIFT))
		{
			mod = 5.0f;
		}
		if (userInput.isKeyPressed(InputKey::W))
		{
			cameraTranslation.z = mod  * -2.0f * (float)_timeDelta;
			pressed = true;
		}
		if (userInput.isKeyPressed(InputKey::S))
		{
			cameraTranslation.z = mod  * 2.0f * (float)_timeDelta;
			pressed = true;
		}
		if (userInput.isKeyPressed(InputKey::A))
		{
			cameraTranslation.x = mod  * -2.0f * (float)_timeDelta;
			pressed = true;
		}
		if (userInput.isKeyPressed(InputKey::D))
		{
			cameraTranslation.x = mod  * 2.0f * (float)_timeDelta;
			pressed = true;
		}
		if (pressed)
		{
			camera->translate(cameraTranslation);
		}
	}
	void CameraController::setSmoothFactor(float _smoothFactor)
	{
		smoothFactor = _smoothFactor;
	}
	float CameraController::getSmoothFactor() const
	{
		return smoothFactor;
	}
}