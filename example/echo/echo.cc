#include "echo.h"

#include <netlib/logging.h>

using namespace netlib;
using std::bind;
using std::string;
using namespace std::placeholders;

EchoServer::EchoServer(EventLoop *loop, const SocketAddress &listen_address):
	server_(loop, listen_address, "EchoServer")
{
	server_.set_connection_callback(bind(&EchoServer::HandleConnection, this, _1));
	server_.set_message_callback(bind(&EchoServer::HandleMessage, this, _1, _2, _3));
}
void EchoServer::HandleConnection(const TcpConnectionPtr &connection)
{
	LOG_INFO("EchoServer - %s -> %s is %s",
	         connection->server_address().ToIpPortString().c_str(),
	         connection->client_address().ToIpPortString().c_str(),
	         (connection->Connected() ? "UP" : "DOWN"));
}
void EchoServer::HandleMessage(const TcpConnectionPtr &connection,
                               Buffer *buffer,
                               TimeStamp time)
{
	string message(buffer->RetrieveAllAsString());
	LOG_INFO("%s echo %d bytes, data received at %s",
	         connection->name().c_str(),
	         static_cast<int>(message.size()),
	         time.ToFormattedTimeString().c_str());
	connection->Send(message);
}

void EchoServer::Start()
{
	server_.Start();
}
