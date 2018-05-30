#include "NetLoadManager.h"
#include <chrono>
#include <iostream>
#include <sstream>
#include "..\Utilities\Utility.h"

typedef std::lock_guard<std::recursive_mutex> lock;

const std::string CRLF = "\r\n";
const std::string charset = "UTF-8";
const std::streamsize bufferSize = 1024;

uint32_t standardTimeout = 40*1000;

NetLoadManager::NetLoadManager()
	: thread(nullptr)
{
}

NetLoadManager::~NetLoadManager()
{
}


NetLoadManager& NetLoadManager::getInstance()
{
	static NetLoadManager manager;
	return manager;
}

NetLoadManager::UpLoad* NetLoadManager::queueUpload(const std::string &host, const std::string &path, const std::vector<std::pair<std::string, std::string>> &files)
{
	lock lock(mutex);
	UpLoad *load = new UpLoad(host, path, files);
	loads.push_back(load);
	updateThread();
	return load;
}

NetLoadManager::DownLoad* NetLoadManager::queueDownload(const std::string &host, const std::string &path)
{
	lock lock(mutex);
	DownLoad *load = new DownLoad(host, path);
	loads.push_back(load);
	updateThread();
	return load;
}

bool NetLoadManager::upload(const std::string &host, const std::string &path, const std::vector<std::pair<std::string, std::string>> &files)
{
	UpLoad load(host, path, files);
	runLoad(&load);
	return load.isSuccess();
}

bool NetLoadManager::download(const std::string &host, const std::string &path, std::string &result)
{
	DownLoad load(host, path);
	runLoad(&load);
	result = load.getResponse();
	return load.isSuccess();
}

NetLoadManager::AbstractLoad* NetLoadManager::nextLoad()
{
	lock lock(mutex);
	return loads.empty() ? nullptr : loads.front();
}

void NetLoadManager::removeLoad(AbstractLoad *load)
{
	lock lock(mutex);
	remove(loads, load);
}

void NetLoadManager::updateThread()
{
	lock lock(mutex);

	if (!loads.empty() && !thread)
	{
		thread = new std::thread([](NetLoadManager *manager)
		{
			AbstractLoad *next;
			while (next = manager->nextLoad())
			{
				manager->runLoad(next);
				manager->removeLoad(next);
			}

			manager->stopThread();
		}, this);
	}
}

void NetLoadManager::stopThread()
{
	lock lock(mutex);
	thread->detach();
	delete thread;
	thread = nullptr;
}



NetLoadManager::UpLoad::UpLoad(const std::string &host, const std::string &path, const std::vector<std::pair<std::string, std::string>> &files)
	: AbstractLoad(host, path, true), files(files)
{
}

NetLoadManager::UpLoad::~UpLoad()
{
}

NetLoadManager::DownLoad::DownLoad(const std::string &host, const std::string &path)
	: AbstractLoad(host, path, false)
{
}

NetLoadManager::DownLoad::~DownLoad()
{
}

NetLoadManager::AbstractLoad::AbstractLoad(const std::string &host, const std::string &path, bool upload)
	: started(false), done(false), percentage(0.0f),
	host(host), path(path), upload(upload)
{
}

NetLoadManager::AbstractLoad::~AbstractLoad()
{
}

const std::string& NetLoadManager::AbstractLoad::getResponse()
{
	lock lock(mutex);
	return response;
}

float NetLoadManager::AbstractLoad::getPercentage()
{
	lock lock(mutex);
	return percentage;
}

bool NetLoadManager::AbstractLoad::isStarted()
{
	lock lock(mutex);
	return started;
}

bool NetLoadManager::AbstractLoad::isDone()
{
	lock lock(mutex);
	return done;
}

bool NetLoadManager::AbstractLoad::isSuccess()
{
	lock lock(mutex);
	return success;
}

void NetLoadManager::AbstractLoad::setResponse(const std::string &response)
{
	lock lock(mutex);
	this->response = response;
}

void NetLoadManager::AbstractLoad::setPercentage(float percentage)
{
	lock lock(mutex);
	this->percentage = percentage;
}

void NetLoadManager::AbstractLoad::setStarted(bool started)
{
	lock lock(mutex);
	this->started = started;
}

void NetLoadManager::AbstractLoad::markDone(bool success)
{
	lock lock(mutex);
	this->done = true;
	this->success = success;
}

void NetLoadManager::runLoad(AbstractLoad *load)
{
	if (load->upload)
	{
		runUpload((UpLoad*)load);
	}
	else
	{
		runDownload((DownLoad*)load);
	}
}

void NetLoadManager::runDownload(DownLoad *load)
{
	load->setStarted(true);

	try
	{
		asio::ip::tcp::iostream stream;
		stream.expires_from_now(std::chrono::milliseconds(standardTimeout));
		stream.connect(load->host, "http");

		if (!stream)
		{
			std::cout << "HTTP Unable to connect: " << stream.error().message() << " to " << load->host << load->path << std::endl;
			load->markDone(false);
			return;
		}
		stream << "GET " << load->path << " HTTP/1.0\r\n";
		stream << "Host: " << load->host << "\r\n";
		stream << "Accept: */*\r\n";
		stream << "Connection: close\r\n\r\n";
		stream.flush();

		// Check that response is OK.
		std::string http_version;
		unsigned int status_code;
		std::string status_message;
		stream >> http_version >> status_code;
		std::getline(stream, status_message);
		if (!stream || http_version.substr(0, 5) != "HTTP/")
		{
			std::cout << "HTTP Invalid response from " << load->host << load->path << std::endl;
			load->markDone(false);
			return;
		}
		if (status_code != 200)
		{
			std::cout << "HTTP Response returned with status code " << status_code << " from " << load->host << load->path << std::endl;
			load->markDone(false);
			return;
		}

		/*********** process response header ************/
		long contentLength = 0;
		std::string line;
		while (std::getline(stream, line) && line != "\r")
		{
			if (0 == line.compare(0, 16, "Content-Length: "))
			{
				try
				{
					contentLength = stol(line.substr(16));
				}
				catch (...)
				{
				}
			}
		}

		/*********** load response body ************/
		std::ostringstream response;
		//os << stream.rdbuf(); // working

		char buffer[bufferSize];
		std::streamsize readBytes = 0;
		auto streamBuffer = stream.rdbuf();

		while (readBytes < contentLength || streamBuffer->in_avail() > 0)
		{
			std::streamsize size = streamBuffer->sgetn(buffer, bufferSize);
			response.write(buffer, size);
			response.flush();

			readBytes += size;
			load->setPercentage((float)readBytes/contentLength);
		}

		load->setResponse(response.str());
		load->markDone(true);
	}
	catch (...)
	{
		std::cout << "ERROR: NetLoadManager::runDownload() from " << load->host << load->path << std::endl;
		load->markDone(false);
	}
}

void NetLoadManager::runUpload(UpLoad *load)
{
	load->setStarted(true);

	auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	std::string boundary = "--"+std::string(to_hex(millis))+"--";

	try
	{
		/********** build request body **********/
		std::stringstream bodyStream;
		std::streampos bodyLength;
		{
			for (std::pair<std::string, std::string> p : load->files)
			{
				const std::string &fileName = p.first;
				const std::string &filePath = p.second;

				bodyStream << "--" << boundary << CRLF;
				bodyStream << "Content-Disposition: form-data; name=\""<<fileName<<"\"; filename=\"" + Util::getPathLastPart(filePath) + "\"" << CRLF;
				bodyStream << "Content-Type: text/plain; charset=" + charset << CRLF;
				bodyStream << CRLF;

				std::vector<char> fileContent = readBinaryFile(filePath.c_str());
				bodyStream.write(fileContent.data(), fileContent.size());
				bodyStream << CRLF;
			}
			bodyStream << "--" + boundary << "--";
			bodyStream.flush();
			bodyLength = bodyStream.tellp();
		}

		/********** start connection **********/
		asio::ip::tcp::iostream stream;
		stream.expires_from_now(std::chrono::minutes(10));

		stream.connect(load->host, "http");

		if (!stream)
		{
			std::cout << "HTTP Unable to connect: " << stream.error().message() << " to " << load->host << load->path << std::endl;
			load->markDone(false);
			return;
		}

		/********** send header **********/
		stream << "POST " << load->path << " HTTP/1.1" << CRLF;
		stream << "Host: " << load->host  << CRLF;
		stream << "Accept: */*"<< CRLF;
		stream << "Content-Length: "<< (size_t)bodyLength << CRLF;
		stream << "Content-Type: multipart/form-data; boundary=" << boundary << CRLF;
		stream << "Connection: close" << CRLF;
		stream << CRLF;

		/********** send body **********/
		//bodyStream.seekp(0);
		//stream << bodyString;
		//stream << bodyStream.rdbuf(); // working
		//stream.flush();

		char buffer[bufferSize];
		std::streamsize sendBytes = 0;

		std::streamsize avail;
		while ((avail = bodyStream.tellp() - bodyStream.tellg()) > 0)
		{
			if (avail > bufferSize)
				avail = bufferSize;

			bodyStream.read(buffer, avail);
			stream.write(buffer, avail);
			stream.flush();

			sendBytes += avail;
			load->setPercentage((float)sendBytes/(float)bodyLength);
		}

		/**********Process Response Header **********/
		std::string http_version;
		unsigned int status_code;
		std::string status_message;

		stream >> http_version >> status_code;
		std::getline(stream, status_message);
		if (!stream || http_version.substr(0, 5) != "HTTP/") // Check that response is OK.
		{
			std::cout << "HTTP Invalid response from " << load->host << load->path << std::endl;
			load->markDone(false);
			return;
		}
		if (status_code != 200)
		{
			std::cout << "HTTP Response returned with status code " << status_code << " from " << load->host << load->path << std::endl;
			load->markDone(false);
			return;
		}

		// load rest of header
		std::string header;
		while (std::getline(stream, header) && header != "\r")
		{
		}

		/***************** Process Response Body *****************/
		std::ostringstream response;
		response << stream.rdbuf();
		load->setResponse(response.str());
		load->markDone(true);
	}
	catch (...)
	{
		std::cout << "ERROR: NetLoadManager::runUpload() from " << load->host << load->path << std::endl;
		load->markDone(false);
	}
}