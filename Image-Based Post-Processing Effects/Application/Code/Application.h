#pragma once
#include <IGameLogic.h>
#include <memory>
#include "CameraController.h"

struct Level;

namespace App
{
	class Application : public IGameLogic
	{
	public:
		explicit Application();
		void init();
		void input(double currentTime, double timeDelta);
		void update(double currentTime, double timeDelta);
		void render();
	private:
		std::shared_ptr<Level> level;
		CameraController cameraController;
	};
}
