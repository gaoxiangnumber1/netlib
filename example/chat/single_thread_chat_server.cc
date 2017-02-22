#include "codec.h"

#include <set>

#include <netlib/event_loop.h>
#include <netlib/logging.h>
#include <netlib/tcp_server.h>

using namespace netlib;
using namespace std::placeholders;
using std::bind;
using std::string;
using std::set;

class ChatServer: public NonCopyable
{
public:
	ChatServer(EventLoop *loop, const SocketAddress &listen_address):
		server_(loop, listen_address, "ChatServer"),
		codec_(bind(&ChatServer::HandleStringMessage, this, _1, _2, _3))
	{
		server_.set_connection_callback(bind(&ChatServer::HandleConnection, this, _1));
		server_.set_message_callback(bind(&Codec::HandleMessage, &codec_, _1, _2, _3));
	}

	void Start()
	{
		server_.Start();
	}

private:
	void HandleConnection(const TcpConnectionPtr &connection)
	{
		bool connected = connection->Connected();
		LOG_INFO("ChatServer - %s -> %s is %s",
		         connection->peer_address().ToIpPortString().c_str(),
		         connection->local_address().ToIpPortString().c_str(),
		         (connected ? "UP" : "DOWN"));
		if(connected == true)
		{
			connection_set_.insert(connection);
		}
		else
		{
			connection_set_.erase(connection);
		}
	}
	void HandleStringMessage(const TcpConnectionPtr &connection,
	                         const string &message,
	                         TimeStamp)
	{
		for(ConnectionSet::iterator it = connection_set_.begin();
		        it != connection_set_.end();
		        ++it)
		{
			codec_.Send(*it, message);
		}
	}

	using ConnectionSet = set<TcpConnectionPtr>;
	TcpServer server_;
	Codec codec_;
	ConnectionSet connection_set_;
};

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		LOG_FATAL("Usage: %s port", argv[0]);
	}
	EventLoop loop;
	SocketAddress listen_address(atoi(argv[1]));
	ChatServer server(&loop, listen_address);
	server.Start();
	loop.Loop();
}
