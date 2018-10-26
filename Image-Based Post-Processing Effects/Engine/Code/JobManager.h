#pragma once
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include "Settings.h"

class IJobManagerJobListener;

class JobManager
{
public:
	class Job;
	typedef std::shared_ptr<Job> SharedJob;
	typedef std::function<void(SharedJob job)> Work;

	SharedJob queue(Work work, Work laterWork = {}, Work cleanWork = {});
	void run(Work work, Work laterWork = {}, Work cleanWork = {});
	static JobManager& getInstance();

	void addListener(IJobManagerJobListener *listener);
	void removeListener(IJobManagerJobListener *listener);

	void check();
	bool usesMultithreading();

private:
	bool m_interrupted;
	std::thread *m_thread;
	std::vector<std::shared_ptr<Job>> m_jobs;
	std::recursive_mutex m_mutex;
	std::condition_variable_any m_hasWork;
	std::vector<IJobManagerJobListener*> m_listeners;

	std::shared_ptr<Setting<bool>> useMultithreadingSetting;
	bool m_useMutlithreading;

	JobManager();
	~JobManager();	
	void interrupt();
	void startThread();
	SharedJob nextJob();
	bool isInterrupted();	
	void runJob(SharedJob job);
	void removeJob(SharedJob job);
	void notifyListenersWithQueued(SharedJob job);
	void notifyListenersWithFinished(SharedJob job);
};

class JobManager::Job
{
public:
	friend JobManager;
	~Job();

	void kill();
	bool isDone();
	bool isKilled();
	bool isStarted();
	bool isSuccess();
	bool isNeedsCleaning();

	void markStarted();
	void markDone(bool success);
	void markNeedsCleaning();

	void setUserData(void *userData);
	void* getUserData();

	void setName(const std::string &name);
	std::string getName();

private:
	Job(Work work, Work laterWork, Work cleanWork);
	
	std::string name;

	bool done;
	bool killed;
	bool started;
	bool success;
	bool needsCleaning;

	const Work work;
	const Work laterWork;
	const Work cleanWork;

	std::recursive_mutex mutex;

	void *userData;
};

class IJobManagerJobListener
{
public:
	virtual ~IJobManagerJobListener() = default;
	virtual void jobManagagerJobQueued(JobManager::SharedJob job, size_t jobCount) = 0;
	virtual void jobManagagerJobFinished(JobManager::SharedJob job, size_t jobCount) = 0;
};