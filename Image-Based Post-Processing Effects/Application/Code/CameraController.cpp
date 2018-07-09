#include "CameraController.h"
#include <cassert>
#include <Input\UserInput.h>
#include <Graphics\Camera.h>
#include <iostream>

namespace App
{
	CameraController::CameraController(std::shared_ptr<Camera> _camera)
		:camera(_camera), userInput(UserInput::getInstance())
	{
	}
	void CameraController::setCamera(std::shared_ptr<Camera> _camera)
	{
		camera = _camera;
	}

	void CameraController::input(double _currentTime, double _timeDelta)
	{
		assert(camera);
		bool gamepad = true;

		if (gamepad)
		{
			const GamepadInputData gamepadInputData = userInput.getGamepadInputData(0);
			static float prevLookAroundXAxis = 0.0f;
			static float prevLookAroundYAxis = 0.0f;

			camera->rotate(glm::vec3(-(gamepadInputData.axisValues[3] - prevLookAroundYAxis), gamepadInputData.axisValues[2] - prevLookAroundXAxis, 0.0) * 0.025f);

			glm::vec3 cameraTranslation;
			bool pressed = false;
			float mod = 1.0f;
			cameraTranslation.z = mod * -5.0f * gamepadInputData.axisValues[1] * (float)_timeDelta;
			cameraTranslation.x = mod * 5.0f * gamepadInputData.axisValues[0] * (float)_timeDelta;
			camera->translate(cameraTranslation);
		}
		else
		{
			const GamepadInputData gamepadInputData = userInput.getGamepadInputData(0);

			if (userInput.isMouseButtonPressed(InputMouse::BUTTON_RIGHT))
			{
				glm::vec2 mouseDelta = userInput.getMousePosDelta();
				camera->rotate(glm::vec3(mouseDelta.y * 0.005f, mouseDelta.x * 0.005f, 0.0));
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

			glm::vec3 cameraTranslation;
			bool pressed = false;
			float mod = 1.0f;

			if (userInput.isKeyPressed(InputKey::LEFT_SHIFT))
			{
				mod = 5.0f;
			}
			if (userInput.isKeyPressed(InputKey::W))
			{
				cameraTranslation.z = mod  * -5.0f * (float)_timeDelta;
				pressed = true;
			}
			if (userInput.isKeyPressed(InputKey::S))
			{
				cameraTranslation.z = mod  * 5.0f * (float)_timeDelta;
				pressed = true;
			}
			if (userInput.isKeyPressed(InputKey::A))
			{
				cameraTranslation.x = mod  * -5.0f * (float)_timeDelta;
				pressed = true;
			}
			if (userInput.isKeyPressed(InputKey::D))
			{
				cameraTranslation.x = mod  * 5.0f * (float)_timeDelta;
				pressed = true;
			}
			if (pressed)
			{
				camera->translate(cameraTranslation);
			}
		}
	}
}