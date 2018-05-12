#pragma once
#include "asioInclude.h"
#include "Connection.h"
#include <vector>

class BaseServer : public NetworkMessageListener, public NetworkDisconnectListener
{
public:
	void start();
	void stop();
	void update();

	BaseServer(unsigned short port);
	virtual ~BaseServer();

	virtual void clientConnected(Connection *connection) = 0;
	virtual void clientDisconnected(Connection *connection) = 0;
	virtual void processMessageBuffer(Connection *connection, ByteBuffer &buffer) = 0;

	virtual void networkMessage(Connection *connection, ByteBuffer &buffer) override;
	virtual void networkDisconnect(Connection *connection) override;

private:
	std::vector<Connection*> connections;
	asio::ip::tcp::acceptor acceptor;
	void queue_accept();
	void handle_accept(Connection *connection, const asio::error_code &error);

};

