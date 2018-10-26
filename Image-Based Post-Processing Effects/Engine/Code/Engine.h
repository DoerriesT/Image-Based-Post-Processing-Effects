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
	static Engine *s_instance;
	IGameLogic &m_gameLogic;
	std::shared_ptr<Window> m_window;
	UserInput &m_userInput;
	SystemManager &m_systemManager;
	std::string m_title;
	double m_lastFrame;
	double m_time;
	double m_timeDelta;
	bool m_shouldShutdown = false;
	double m_lastFpsMeasure;
	double m_fps;

	std::mutex m_mutex;
	std::vector<std::function<void()>> m_functionQueue;

	std::shared_ptr<Setting<bool>> m_showFps;

	void gameLoop();
	void input(double _currentTime, double _timeDelta);
	void update(double _currentTime, double _timeDelta);
	void render();
};