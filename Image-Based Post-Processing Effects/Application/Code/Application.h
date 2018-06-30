#pragma once
#include <IGameLogic.h>
#include <memory>
#include "CameraController.h"
#include <Gui\Gui.h>

struct Level;

namespace App
{
	class Application : public IGameLogic, public IGuiEventListener
	{
	public:
		explicit Application();
		void init();
		void input(double time, double timeDelta) override;
		void update(double time, double timeDelta) override;
		void render() override;
		void guiEventNotification(struct GuiEvent &_event) override;

	private:
		std::shared_ptr<Level> level;
		CameraController cameraController;
		Gui gui;
		std::unique_ptr<GuiLayout> optionsGui;

		std::shared_ptr<Setting<int>> shadowQuality;
		std::shared_ptr<Setting<int>> uiSizeOffset;
		std::shared_ptr<Setting<int>> anisotropicFiltering;
		std::shared_ptr<Setting<bool>> bloomEnabled;
		std::shared_ptr<Setting<bool>> fxaaEnabled;
		std::shared_ptr<Setting<bool>> lensFlaresEnabled;
		std::shared_ptr<Setting<int>> ambientOcclusion;
		std::shared_ptr<Setting<bool>> vsync;
		std::shared_ptr<Setting<int>> windowWidth;
		std::shared_ptr<Setting<int>> windowHeight;
		std::shared_ptr<Setting<int>> windowMode;
		std::shared_ptr<Setting<int>> motionBlur;
		std::shared_ptr<Setting<int>> ssaoKernelSize;
		std::shared_ptr<Setting<double>> ssaoRadius;
		std::shared_ptr<Setting<double>> ssaoStrength;
		std::shared_ptr<Setting<int>> hbaoDirections;
		std::shared_ptr<Setting<int>> hbaoSteps;
		std::shared_ptr<Setting<double>> hbaoStrength;
		std::shared_ptr<Setting<double>> hbaoRadius;
		std::shared_ptr<Setting<double>> hbaoMaxRadiusPixels;
		std::shared_ptr<Setting<double>> hbaoAngleBias;

		void initGuiData();
	};
}
