#pragma once
#include <IGameLogic.h>
#include <memory>
#include <vector>
#include "CameraController.h"
#include <AntTweakBar.h>
#include <Window\GLFW\IInputListener.h>
#include <Window\GLFW\IWindowResizeListener.h>
#include <Settings.h>

#define GETTER_FUNC_DECL(name) void TW_CALL name##GetCallback(void *value, void *clientData);
#define SETTER_FUNC_DECL(name) void TW_CALL name##SetCallback(const void *value, void *clientData);
#define COMBINED_FUNC_DECL(name) friend GETTER_FUNC_DECL(name) friend SETTER_FUNC_DECL(name)

struct Level;

namespace App
{
	class Application : public IGameLogic, public IInputListener, public IWindowResizeListener
	{
		COMBINED_FUNC_DECL(windowResolution)
			COMBINED_FUNC_DECL(shadowQuality)
			COMBINED_FUNC_DECL(shadowQuality)
			COMBINED_FUNC_DECL(anisotropicFiltering)
			COMBINED_FUNC_DECL(bloomEnabled)
			COMBINED_FUNC_DECL(fxaaEnabled)
			COMBINED_FUNC_DECL(smaaEnabled)
			COMBINED_FUNC_DECL(smaaTemporalAA)
			COMBINED_FUNC_DECL(lensFlaresEnabled)
			COMBINED_FUNC_DECL(ambientOcclusion)
			COMBINED_FUNC_DECL(vsync)
			COMBINED_FUNC_DECL(windowWidth)
			COMBINED_FUNC_DECL(windowHeight)
			COMBINED_FUNC_DECL(windowMode)
			COMBINED_FUNC_DECL(motionBlur)
			COMBINED_FUNC_DECL(depthOfField)
			COMBINED_FUNC_DECL(ssaoKernelSize)
			COMBINED_FUNC_DECL(ssaoRadius)
			COMBINED_FUNC_DECL(ssaoStrength)
			COMBINED_FUNC_DECL(hbaoDirections)
			COMBINED_FUNC_DECL(hbaoSteps)
			COMBINED_FUNC_DECL(hbaoStrength)
			COMBINED_FUNC_DECL(hbaoRadius)
			COMBINED_FUNC_DECL(hbaoMaxRadiusPixels)
			COMBINED_FUNC_DECL(hbaoAngleBias)
			COMBINED_FUNC_DECL(colors)
			COMBINED_FUNC_DECL(bouncing)

	public:
		explicit Application();
		void init();
		void input(double time, double timeDelta) override;
		void update(double time, double timeDelta) override;
		void render() override;

		void onKey(int _key, int _action) override;
		void onChar(int _charKey)  override;
		void onMouseButton(int _mouseButton, int _action) override;
		void onMouseMove(double _x, double _y) override;
		void onMouseEnter(bool _entered) override;
		void onMouseScroll(double _xOffset, double _yOffset) override;
		void gamepadUpdate(const std::vector<Gamepad> *_gamepads) override;
		void onResize(unsigned int width, unsigned int height) override;

	private:
		std::shared_ptr<Level> level;
		CameraController cameraController;
		TwBar *settingsTweakBar;
		std::vector<std::string> afOptionStrings;
		std::vector<std::string> resolutionOptionStrings;
		std::string fpsStr;
		std::string fpsAvgStr;
		std::string fpsWorstStr;
		std::string frameTimeStr;
		std::string frameTimeAvgStr;
		std::string frameTimeWorstStr;
		bool colors;
		bool bouncing;

		std::shared_ptr<Setting<int>> shadowQuality;
		std::shared_ptr<Setting<int>> uiSizeOffset;
		std::shared_ptr<Setting<int>> anisotropicFiltering;
		std::shared_ptr<Setting<bool>> bloomEnabled;
		std::shared_ptr<Setting<bool>> fxaaEnabled;
		std::shared_ptr<Setting<bool>> smaaEnabled;
		std::shared_ptr<Setting<bool>> smaaTemporalAA;
		std::shared_ptr<Setting<bool>> lensFlaresEnabled;
		std::shared_ptr<Setting<int>> ambientOcclusion;
		std::shared_ptr<Setting<bool>> vsync;
		std::shared_ptr<Setting<int>> windowWidth;
		std::shared_ptr<Setting<int>> windowHeight;
		std::shared_ptr<Setting<int>> windowMode;
		std::shared_ptr<Setting<int>> motionBlur;
		std::shared_ptr<Setting<int>> depthOfField;
		std::shared_ptr<Setting<int>> ssaoKernelSize;
		std::shared_ptr<Setting<double>> ssaoRadius;
		std::shared_ptr<Setting<double>> ssaoStrength;
		std::shared_ptr<Setting<int>> hbaoDirections;
		std::shared_ptr<Setting<int>> hbaoSteps;
		std::shared_ptr<Setting<double>> hbaoStrength;
		std::shared_ptr<Setting<double>> hbaoRadius;
		std::shared_ptr<Setting<double>> hbaoMaxRadiusPixels;
		std::shared_ptr<Setting<double>> hbaoAngleBias;

		//void initGuiData();
	};
}
