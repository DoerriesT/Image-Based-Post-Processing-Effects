#pragma once
#include <string>
#include <mutex>
#include <memory>
#include "Settings.h"

class Window;
class SystemManager;
class IGameLogic;
class UserInput;

class Engine
{
public:
	explicit Engine(const std::string &_title, IGameLogic &_gameLogic);
	~Engine();
	void start();
	void shutdown();
	static double getCurrentTime();
	static double getCurrentTimeDelta();
	static double getCurrentFps();
	static Engine* getInstance();
	Window* getWindow();

	static void runLater(std::function<void()> function);
	static bool isFunctionQueueEmpty();

	int getMaxAnisotropicFiltering();

private:
	IGameLogic &gameLogic;
	std::shared_ptr<Window> window;
	UserInput &userInput;
	SystemManager &systemManager;
	std::string title;
	static double currentTime;
	static double currentTimeDelta;
	static Engine *instance;
	double lastFrame;
	bool shouldShutdown = false;
	double lastFpsMeasure;
	double fps;

	std::mutex mutex;
	std::vector<std::function<void()>> functionQueue;

	std::shared_ptr<Setting<bool>> showFps;

	void gameLoop();
	void input(double _currentTime, double _timeDelta);
	void update(double _currentTime, double _timeDelta);
	void render();
};