#include "JobManager.h"
#include "Utilities\Utility.h"
#include "IGameLogic.h"
#include "Engine.h"

typedef std::unique_lock<std::recursive_mutex> unique_lock;
typedef std::unique_lock<std::recursive_mutex> lock_guard;

JobManager::JobManager()
	: thread(nullptr), interrupted(false)
{
	useMultithreadingSetting = SettingsManager::getInstance().getBoolSetting("misc", "use_multithreading", true);
	useMultithreadingSetting->addListener([&](const bool &value)
	{
		lock_guard lock(mutex);
		useMutlithreading = value;
	});

	useMutlithreading = useMultithreadingSetting->get();

	if (useMutlithreading)
	{
		startThread();
	}
}

JobManager::~JobManager()
{
	for (SharedJob job : jobs)
	{
		job->kill();
	}

	if (thread)
	{
		interrupt();
		hasWork.notify_one();
		thread->join();
		delete thread;
	}
}

JobManager::SharedJob JobManager::nextJob()
{
	lock_guard lock(mutex);
	for (SharedJob job : jobs)
	{
		if (!job->isStarted() || job->isDone() || job->isKilled())
		{
			return job;
		}
	}
	return nullptr;
}

JobManager& JobManager::getInstance()
{
	static JobManager manager;
	return manager;
}

JobManager::SharedJob JobManager::queue(JobManager::Work work, JobManager::Work laterWork, JobManager::Work cleanWork)
{
	if (useMutlithreading)
	{
		lock_guard lock(mutex);
		auto job = SharedJob(new Job(work, laterWork, cleanWork));
		jobs.push_back(job);
		notifyListenersWithQueued(job);

		hasWork.notify_one();
		return job;
	}
	else
	{
		run(work, laterWork, cleanWork);
		notifyListenersWithFinished(nullptr);
		return nullptr;
	}
}

void JobManager::run(Work work, Work laterWork, Work cleanWork)
{
	auto job = SharedJob(new Job(work, {}, {}));
	assert(work);
	try
	{
		work(job);

		if (!job->isDone() && !job->isKilled() && laterWork)
		{
			laterWork(job);
		}
	}
	catch (...) {}

	if (cleanWork)
	{
		try
		{
			cleanWork(job);
		}
		catch (...) {}
	}
}

void JobManager::removeJob(SharedJob job)
{
	if (job->cleanWork && job->isNeedsCleaning())
	{
		try
		{
			job->cleanWork(job);
		}
		catch (...) {}
	}

	lock_guard lock(mutex);
	remove(jobs, job);
}

void JobManager::startThread()
{
	lock_guard lock(mutex);
	if (!thread)
	{
		thread = new std::thread([this]()
		{
			while (!isInterrupted())
			{
				SharedJob next;
				{
					unique_lock lock(mutex);

					while (!(next = nextJob()) && !isInterrupted())
					{
						if (jobs.empty())
						{
							hasWork.wait(lock);
						}
						else
						{
							hasWork.wait_for(lock, std::chrono::seconds(4));
						}
					}
					if (isInterrupted())
					{
						break;
					}
				}
				if (next->isKilled() || next->isDone())
				{
					notifyListenersWithFinished(next);
					removeJob(next);
				}
				else
				{
					next->markStarted();
					runJob(next);
				}
			}
		});
	}
}

bool JobManager::usesMultithreading()
{
	lock_guard lock(mutex);
	return useMutlithreading;
}

void JobManager::notifyListenersWithQueued(SharedJob job)
{
	Engine::runLater([this, job]()
	{
		lock_guard lock(mutex);
		const size_t jobCount = jobs.size();

		for (auto listener : listeners)
		{
			listener->jobManagagerJobQueued(job, jobCount);
		}
	});
}

void JobManager::notifyListenersWithFinished(SharedJob job)
{
	Engine::runLater([this, job]()
	{
		lock_guard lock(mutex);
		const size_t jobCount = jobs.size();

		auto listeners = this->listeners;
		for (auto listener : listeners)
		{
			listener->jobManagagerJobFinished(job, jobCount);
		}
	});
}

void JobManager::addListener(IJobManagerJobListener *listener)
{
	lock_guard lock(mutex);
	if (!contains(listeners, listener))
	{
		listeners.push_back(listener);
	}
}

void JobManager::removeListener(IJobManagerJobListener *listener)
{
	lock_guard lock(mutex);
	remove(listeners, listener);
}

void JobManager::check()
{
	hasWork.notify_one();
}

void JobManager::interrupt()
{
	lock_guard lock(mutex);
	interrupted = true;
	hasWork.notify_all();
}

bool JobManager::isInterrupted()
{
	lock_guard lock(mutex);
	return interrupted;
}

void JobManager::runJob(SharedJob job)
{
	try
	{
		job->work(job);
		job->markNeedsCleaning();
	}
	catch (...) {}

	if (!job->isDone() && !job->isKilled())
	{
		if (job->laterWork)
		{
			Engine::runLater([this, job]()
			{
				if (!job->isKilled())
				{
					try
					{
						job->laterWork(job);
					}
					catch (...) {}
				}
				job->markDone(true);
			});
		}
	}
}


JobManager::Job::Job(Work work, Work laterWork, Work cleanWork)
	: started(false), done(false), killed(false), needsCleaning(false),
	work(work), laterWork(laterWork), cleanWork(cleanWork)
{
	assert(work);
}

JobManager::Job::~Job()
{
}

bool JobManager::Job::isStarted()
{
	lock_guard lock(mutex);
	return started;
}

bool JobManager::Job::isDone()
{
	lock_guard lock(mutex);
	return done;
}

bool JobManager::Job::isSuccess()
{
	lock_guard lock(mutex);
	return success;
}

bool JobManager::Job::isNeedsCleaning()
{
	lock_guard lock(mutex);
	return needsCleaning;
}

void JobManager::Job::setUserData(void *userData)
{
	lock_guard lock(mutex);
	assert(userData);
	this->userData = userData;
}

void * JobManager::Job::getUserData()
{
	lock_guard lock(mutex);
	assert(userData);
	return userData;
}

void JobManager::Job::setName(const std::string &name)
{
	lock_guard lock(mutex);
	this->name = name;
}

std::string JobManager::Job::getName()
{
	lock_guard lock(mutex);
	return name;
}

bool JobManager::Job::isKilled()
{
	lock_guard lock(mutex);
	return killed;
}

void JobManager::Job::kill()
{
	lock_guard lock(mutex);
	killed = true;
}

void JobManager::Job::markStarted()
{
	lock_guard lock(mutex);
	started = true;
}

void JobManager::Job::markNeedsCleaning()
{
	lock_guard lock(mutex);
	needsCleaning = true;
}

void JobManager::Job::markDone(bool success)
{
	lock_guard lock(mutex);
	if (!done)
	{
		done = true;
		this->success = success;
		JobManager::getInstance().check();
	}
}

