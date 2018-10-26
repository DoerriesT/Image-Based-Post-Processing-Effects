#include "JobManager.h"
#include "Utilities\ContainerUtility.h"
#include "IGameLogic.h"
#include "Engine.h"
#include <cassert>

typedef std::unique_lock<std::recursive_mutex> unique_lock;
typedef std::unique_lock<std::recursive_mutex> lock_guard;

JobManager::JobManager()
	: m_thread(nullptr), m_interrupted(false)
{
	useMultithreadingSetting = SettingsManager::getInstance().getBoolSetting("misc", "use_multithreading", true);
	useMultithreadingSetting->addListener([&](bool value)
	{
		lock_guard lock(m_mutex);
		m_useMutlithreading = value;
	});

	m_useMutlithreading = useMultithreadingSetting->get();

	if (m_useMutlithreading)
	{
		startThread();
	}
}

JobManager::~JobManager()
{
	for (SharedJob job : m_jobs)
	{
		job->kill();
	}

	if (m_thread)
	{
		interrupt();
		m_hasWork.notify_one();
		m_thread->join();
		delete m_thread;
	}
}

JobManager::SharedJob JobManager::nextJob()
{
	lock_guard lock(m_mutex);
	for (SharedJob job : m_jobs)
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
	if (m_useMutlithreading)
	{
		lock_guard lock(m_mutex);
		auto job = SharedJob(new Job(work, laterWork, cleanWork));
		m_jobs.push_back(job);
		notifyListenersWithQueued(job);

		m_hasWork.notify_one();
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

	lock_guard lock(m_mutex);
	ContainerUtility::remove(m_jobs, job);
}

void JobManager::startThread()
{
	lock_guard lock(m_mutex);
	if (!m_thread)
	{
		m_thread = new std::thread([this]()
		{
			while (!isInterrupted())
			{
				SharedJob next;
				{
					unique_lock lock(m_mutex);

					while (!(next = nextJob()) && !isInterrupted())
					{
						if (m_jobs.empty())
						{
							m_hasWork.wait(lock);
						}
						else
						{
							m_hasWork.wait_for(lock, std::chrono::seconds(4));
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
	lock_guard lock(m_mutex);
	return m_useMutlithreading;
}

void JobManager::notifyListenersWithQueued(SharedJob job)
{
	Engine::runLater([this, job]()
	{
		lock_guard lock(m_mutex);
		const size_t jobCount = m_jobs.size();

		for (auto listener : m_listeners)
		{
			listener->jobManagagerJobQueued(job, jobCount);
		}
	});
}

void JobManager::notifyListenersWithFinished(SharedJob job)
{
	Engine::runLater([this, job]()
	{
		lock_guard lock(m_mutex);
		const size_t jobCount = m_jobs.size();

		auto listeners = this->m_listeners;
		for (auto listener : listeners)
		{
			listener->jobManagagerJobFinished(job, jobCount);
		}
	});
}

void JobManager::addListener(IJobManagerJobListener *listener)
{
	lock_guard lock(m_mutex);
	if (!ContainerUtility::contains(m_listeners, listener))
	{
		m_listeners.push_back(listener);
	}
}

void JobManager::removeListener(IJobManagerJobListener *listener)
{
	lock_guard lock(m_mutex);
	ContainerUtility::remove(m_listeners, listener);
}

void JobManager::check()
{
	m_hasWork.notify_one();
}

void JobManager::interrupt()
{
	lock_guard lock(m_mutex);
	m_interrupted = true;
	m_hasWork.notify_all();
}

bool JobManager::isInterrupted()
{
	lock_guard lock(m_mutex);
	return m_interrupted;
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

