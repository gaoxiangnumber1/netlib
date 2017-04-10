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
	ChatServer(EventLoop *loop, const SocketAddress &listen_address, int thread_number):
		server_(loop, listen_address, "ChatServer", thread_number),
		codec_(bind(&ChatServer::HandleStringMessage, this, _1, _2, _3)),
		mutex_()
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
		         connection->client_address().ToIpPortString().c_str(),
		         connection->server_address().ToIpPortString().c_str(),
		         (connected ? "UP" : "DOWN"));
		MutexLockGuard lock(mutex_);
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
		MutexLockGuard lock(mutex_);
		for(ConnectionSet::iterator it = connection_set_.begin();
		        it != connection_set_.end();
		        ++it)
		{
			// FIXME: when there has many clients, some clients receive strange buffer
			// content, though the buffer->ReadableByte() is still right, the message length
			// at header is wrong. Why? Maybe the buffer is destructed?
			codec_.Send(*it, message);
		}
	}

	using ConnectionSet = set<TcpConnectionPtr>;
	TcpServer server_;
	Codec codec_;
	MutexLock mutex_;
	ConnectionSet connection_set_;
};

int main(int argc, char **argv)
{
	if(argc != 3)
	{
		LOG_FATAL("Usage: %s port thread_number", argv[0]);
	}
	EventLoop loop;
	SocketAddress listen_address(atoi(argv[1]));
	ChatServer server(&loop, listen_address, atoi(argv[2]));
	server.Start();
	loop.Loop();
}
