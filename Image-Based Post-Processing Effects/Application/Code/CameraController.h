#pragma once
#include <memory>

class Camera;
class UserInput;

namespace App
{
	class CameraController
	{
	public:
		explicit CameraController(std::shared_ptr<Camera> _camera = nullptr);
		void setCamera(std::shared_ptr<Camera> _camera);
		void input(double _currentTime, double _timeDelta);

	private:
		std::shared_ptr<Camera> camera;
		UserInput &userInput;
	};
}