#pragma once
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include "asioInclude.h"

class NetLoadManager
{
public:
	class AbstractLoad;
	class UpLoad;
	class DownLoad;

	static NetLoadManager& getInstance();

	UpLoad* queueUpload(const std::string &host, const std::string &path, const std::vector<std::pair<std::string, std::string>> &files);

	DownLoad* queueDownload(const std::string &host, const std::string &path);

	bool upload(const std::string &host, const std::string &path, const std::vector<std::pair<std::string, std::string>> &files);

	//fills the result parameter with the contents of a http file, returns false if the download failed. example usage: downloadHTTPFileContent("www.google.com", "/index.html", result)
	bool download(const std::string &host, const std::string &path, std::string &result);

private:
	std::recursive_mutex mutex;
	std::thread *thread;

	NetLoadManager();
	~NetLoadManager();
	std::vector<AbstractLoad*> loads;

	AbstractLoad* nextLoad();
	void removeLoad(AbstractLoad *load);

	void updateThread();
	void stopThread();

	void runLoad(AbstractLoad *load);
	void runUpload(UpLoad *load);
	void runDownload(DownLoad *load);
};

class NetLoadManager::AbstractLoad
{
public:
	friend NetLoadManager;

	const std::string& getResponse();
	float getPercentage();
	bool isStarted();
	bool isDone();
	bool isSuccess();

	const std::string host;
	const std::string path;
	const bool upload;

protected:
	AbstractLoad(const std::string &host, const std::string &path, bool upload);
	virtual ~AbstractLoad();
	std::recursive_mutex mutex;

	void setResponse(const std::string& result);
	void setPercentage(float percentage);
	void setStarted(bool started);
	void markDone(bool success);

private:
	std::string response;
	float percentage;
	bool started;
	bool done;
	bool success;
};

class NetLoadManager::UpLoad : public NetLoadManager::AbstractLoad
{
public:	
	friend NetLoadManager;
	~UpLoad();

private:
	const std::vector<std::pair<std::string, std::string>> files;
	UpLoad(const std::string &host, const std::string &path, const std::vector<std::pair<std::string, std::string>> &files);

};

class NetLoadManager::DownLoad : public NetLoadManager::AbstractLoad
{
public:
	friend NetLoadManager;
	~DownLoad();

private:
	DownLoad(const std::string &host, const std::string &path);
};