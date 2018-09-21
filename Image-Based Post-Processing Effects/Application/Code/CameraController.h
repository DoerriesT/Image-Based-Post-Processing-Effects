#pragma once
#include <memory>
#include <glm/vec2.hpp>

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
		void setSmoothFactor(float _smoothFactor);
		float getSmoothFactor() const;

	private:
		std::shared_ptr<Camera> camera;
		UserInput &userInput;
		bool grabbedMouse;
		glm::vec2 mouseHistory;
		float smoothFactor;
	};
}