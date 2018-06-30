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
	static Engine *getInstance();
	static double getTime();
	static double getTimeDelta();
	static double getFps();
	static void runLater(std::function<void()> function);
	static bool isFunctionQueueEmpty();

	explicit Engine(const std::string &_title, IGameLogic &_gameLogic);
	~Engine();
	void start();
	void shutdown();
	Window *getWindow();
	int getMaxAnisotropicFiltering();

private:
	static Engine *instance;
	IGameLogic &gameLogic;
	std::shared_ptr<Window> window;
	UserInput &userInput;
	SystemManager &systemManager;
	std::string title;
	double lastFrame;
	double time;
	double timeDelta;
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