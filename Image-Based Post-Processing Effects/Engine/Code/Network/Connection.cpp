#include "Connection.h"
#include <iostream>
#include <functional>
#include "DebugMacro.h"
#include "ByteBuffer.h"


using asio::ip::tcp;
asio::io_service Connection::io_service;

Connection::Connection() : socket_(io_service), inBuffer(), outBuffer(), writeQueued(false), connected(false), messageListener(nullptr), disconnectListener(nullptr), attachment(nullptr)
{
}

Connection::~Connection()
{
	stop();
}

void Connection::queue_read()
{
	char *buf = new char[512];
	auto buffer = asio::buffer(buf, 512);

	const auto handle = std::bind(&Connection::handle_read, this,
								  buf,
								  std::placeholders::_1, std::placeholders::_2);

	//asio::async_read(_socket, buffer, handle);
	socket_.async_read_some(buffer, handle);
}

void Connection::queue_write()
{
	if (!writeQueued)
	{
		outBuffer.rewind();
		if (outBuffer.remaining() > 0)
		{
			size_t s = outBuffer.size();
			const char* data = outBuffer.content();
			auto asioBuffer = asio::buffer(data, s);

			asio::async_write(socket_, asioBuffer,
							  std::bind(&Connection::handle_write, this,
										std::placeholders::_1, std::placeholders::_2));
			writeQueued = true;
		}
	}
}

bool Connection::connect(const char *host, unsigned short port)
{
	try
	{
		if (!connected)
		{
			asio::error_code error;
			//connect to ip
			asio::ip::address ipAddress = asio::ip::address::from_string(host, error);

			if (!error)
			{
				asio::ip::tcp::endpoint endpoint(ipAddress, port);
				socket_.connect(endpoint, error);

				if (!error)
				{
					return true;
				}
			}

			//connect to url
			asio::ip::tcp::resolver resolver(io_service);
			asio::ip::tcp::resolver::query query(host, std::to_string(port));
			asio::connect(socket_, resolver.resolve(query), error);
			return !error;
		}
	}
	catch (const std::exception&)
	{
	}
	return false;

}

void Connection::start()
{
	if (!connected)
	{
		queue_read();
		connected = true;
	}
	else
	{
		throw std::exception("You can't start a Connection that is already running.");
	}
}

void Connection::stop()
{
	if (connected)
	{
		socket_.close();
		connected = false;
		if (disconnectListener)
		{
			disconnectListener->networkDisconnect(this);
		}
	}
}

bool Connection::isConnected() const
{
	return connected;
}

void Connection::setMessageListener(NetworkMessageListener *messageListener)
{
	this->messageListener = messageListener;
}

void Connection::setDisconnectListener(NetworkDisconnectListener *disconnectListener)
{
	this->disconnectListener = disconnectListener;
}

void Connection::update()
{
	io_service.poll_one();
}

void Connection::send(const ByteBuffer &buffer)
{
	outBuffer.head();
	outBuffer.put(buffer);
	queue_write();
}

void Connection::send(const char* buf, const size_t size)
{
	outBuffer.head();
	outBuffer.put(buf, size);
	queue_write();
}

void Connection::handle_write(const asio::error_code& error, const size_t &bytesWritten)
{
	std::cout << "handle_write() written bytes: " << bytesWritten << std::endl;

	writeQueued = false;
	if (!error)
	{
		outBuffer.position(bytesWritten);
		outBuffer.discard();

		queue_write();
	}
	else
	{
		DBG_LOG("ERROR %s", error.message().c_str());
		stop();
	}
}

void Connection::handle_read(const char *buf, const asio::error_code &error, const size_t &bytesRead)
{
	if (!error)
	{
		std::cout << "handle_read() incomming size: " << bytesRead << " -> \"";
		//std::cout.write(buf, bytesRead);
		//std::cout << "\" -> ";
		for (size_t i = 0; i<bytesRead; ++i)
		{
			std::cout << (int)buf[i] << " ";
		}
		std::cout << std::endl;

		inBuffer.head();
		inBuffer.put<char>(buf, bytesRead);

		if (messageListener)
		{
			messageListener->networkMessage(this, inBuffer);
		}
		queue_read();
	}
	else
	{
		DBG_LOG("ERROR %s", error.message().c_str());
		stop();
	}
	delete[] buf;

	//if (error != asio::error::eof) //client closed connection
}
