#include "echo.h"
#include <boost/bind.hpp>
#include <muduo/base/Logging.h>

//using directives inject all the names from a namespace. If an application use using
//directives to make many libraries it used visible, then the global namespace pollution
//problem appears.
//It is better to use a using declaration for each namespace name used in the program.
//Doing so reduces the number of names injected into the namespace. Ambiguity errors
//caused by using declarations are detected at the point of declaration, not use, and so
//are easier to find and fix.

using muduo::net::EventLoop;
using muduo::net::InetAddress;
using boost::bind;
using muduo::net::TcpConnectionPtr;
using muduo::net::Buffer;
using muduo::Timestamp;
using muduo::string;

EchoServer::EchoServer(EventLoop *loop,
                       const InetAddress &listen_address)
	: server_(loop, listen_address, "EchoServer")
{
	// Set callback functions.
	server_.setConnectionCallback(bind(&EchoServer::OnConnection, this, _1));
	server_.setMessageCallback(bind(&EchoServer::OnMessage, this, _1, _2, _3));
}

void EchoServer::Start()
{
	server_.start(); // Invoke TcpServer::start().
}

void EchoServer::OnConnection(const TcpConnectionPtr &connection)
{
	LOG_INFO << "EchoServer - " << connection->peerAddress().toIpPort() << " -> "
	         << connection->localAddress().toIpPort() << " is "
	         << (connection->connected() ? "UP" : "DOWN"); // Print IP_address:Port
}

void EchoServer::OnMessage(const TcpConnectionPtr &connection,
                           Buffer* buffer,
                           Timestamp time)
{
	string message(buffer->retrieveAllAsString()); // Retrieve all bytes from buffer.
	LOG_INFO << connection->name() << " echo " << message.size() << " bytes, "
	         << "data received at " << time.toString();
	connection->send(message); // Echo message as it comes.
}
