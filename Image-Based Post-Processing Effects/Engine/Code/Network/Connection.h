#pragma once

#include "asioInclude.h"
#include <vector>
#include "ByteBuffer.h"

class Connection;

class NetworkMessageListener
{
public:
	virtual ~NetworkMessageListener() = default;
	virtual void networkMessage(Connection *connection, ByteBuffer &buffer) = 0;
};

class NetworkDisconnectListener
{
public:
	virtual ~NetworkDisconnectListener() = default;
	virtual void networkDisconnect(Connection *connection) = 0;
};

class Connection
{
public:
	Connection();
	~Connection();
	
	// an object can be attached to every connection
	void *attachment;

	static asio::io_service io_service;

	void send(const ByteBuffer &buffer);
	void send(const char *buf, const size_t size);
	void queue_read();
	void queue_write();

	//connects to given endpoint
	bool connect(const char *host, unsigned short port);
	//needs to be called after a connection is established
	void start();
	//closes the connection
	void stop();

	bool isConnected() const;
	void setMessageListener(NetworkMessageListener *messageListener);
	void setDisconnectListener(NetworkDisconnectListener *disconnectListener);

	inline asio::ip::tcp::socket& socket()
	{
		return socket_;
	};

	static void update();


private:
	ByteBuffer inBuffer;
	ByteBuffer outBuffer;

	NetworkMessageListener *messageListener;
	NetworkDisconnectListener *disconnectListener;
	asio::ip::tcp::socket socket_;

	bool writeQueued;
	bool connected;

	void handle_read(const char *buf, const asio::error_code &error, const size_t &bytesRead);
	void handle_write(const asio::error_code &error, const size_t &bytesWritten);

};


