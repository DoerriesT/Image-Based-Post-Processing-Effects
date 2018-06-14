#include "CameraController.h"
#include <cassert>
#include <UserInput.h>
#include <Graphics\Camera.h>

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

		if (userInput.isMouseButtonPressed(INPUT_MOUSE_BUTTON_RIGHT))
		{
			glm::vec2 mouseDelta = userInput.getMousePosDelta();
			camera->rotate(glm::vec3(mouseDelta.y * 0.002f, mouseDelta.x * 0.002f, 0.0));
		}
		if (userInput.isKeyPressed(INPUT_KEY_UP))
		{
			camera->rotate(glm::vec3(-_timeDelta, 0.0f , 0.0));
		}
		if (userInput.isKeyPressed(INPUT_KEY_DOWN))
		{
			camera->rotate(glm::vec3(_timeDelta, 0.0f, 0.0));
		}
		if (userInput.isKeyPressed(INPUT_KEY_LEFT))
		{
			camera->rotate(glm::vec3(0.0f, -_timeDelta,  0.0));
		}
		if (userInput.isKeyPressed(INPUT_KEY_RIGHT))
		{
			camera->rotate(glm::vec3(0.0f, _timeDelta, 0.0));
		}

		glm::vec3 cameraTranslation;
		bool pressed = false;
		float mod = 1.0f;

		if (userInput.isKeyPressed(INPUT_KEY_LEFT_SHIFT))
		{
			mod = 5.0f;
		}
		if (userInput.isKeyPressed(INPUT_KEY_W))
		{
			cameraTranslation.z = mod  * -5.0f * (float)_timeDelta;
			pressed = true;
		}
		if (userInput.isKeyPressed(INPUT_KEY_S))
		{
			cameraTranslation.z = mod  * 5.0f * (float)_timeDelta;
			pressed = true;
		}
		if (userInput.isKeyPressed(INPUT_KEY_A))
		{
			cameraTranslation.x = mod  * -5.0f * (float)_timeDelta;
			pressed = true;
		}
		if (userInput.isKeyPressed(INPUT_KEY_D))
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