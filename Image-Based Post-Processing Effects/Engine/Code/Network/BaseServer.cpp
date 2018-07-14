#include "BaseServer.h"
#include <iostream>
#include "DebugMacro.h"
#include <functional>
#include "Connection.h"
#include "Utilities\ContainerUtility.h"
#include "Connection.h"

using asio::ip::tcp;

BaseServer::BaseServer(unsigned short port) : acceptor(Connection::io_service, tcp::endpoint(tcp::v4(), port))
{
}


BaseServer::~BaseServer()
{
	for (Connection *c : connections)
	{
		c->stop();
		delete c;
	}
}

void BaseServer::networkMessage(Connection *connection, ByteBuffer &buffer)
{
	processMessageBuffer(connection, buffer);
}

void BaseServer::networkDisconnect(Connection *connection)
{
	ContainerUtility::remove(connections, connection);
	clientDisconnected(connection);

	delete connection;
}
void BaseServer::queue_accept()
{
	DBG_LOG("");

	Connection *emptyConnection = new Connection();
	//add all connection to deletion list

	//acceptor_.async_accept(new_connection->socket(),
		//boost::bind(&tcp_server::handle_accept, this, new_connection,
			//asio::placeholders::error));

	auto handle = std::bind(&BaseServer::handle_accept,
							this, emptyConnection, std::placeholders::_1);

	acceptor.async_accept(emptyConnection->socket(), handle);
}


void BaseServer::handle_accept(Connection *connection, const asio::error_code& error)
{
	if (!error)
	{
		connection->setDisconnectListener(this);
		connection->setMessageListener(this);
		connections.push_back(connection);

		connection->start();

		clientConnected(connection);
	}

	queue_accept();
}

void BaseServer::update()
{
	Connection::update();
}

void BaseServer::start()
{
	queue_accept();
}

void BaseServer::stop()
{
}




