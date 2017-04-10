#include "codec.h"

#include <set>

#include <netlib/event_loop.h>
#include <netlib/logging.h>
#include <netlib/tcp_server.h>

using namespace netlib;
using namespace std::placeholders;
using std::shared_ptr;
using std::bind;
using std::string;
using std::set;

class ChatServer: public NonCopyable
{
public:
	ChatServer(EventLoop *loop, const SocketAddress &listen_address, int thread_number):
		server_(loop, listen_address, "ChatServer", thread_number),
		codec_(bind(&ChatServer::HandleStringMessage, this, _1, _2, _3)),
		mutex_(),
		connection_set_ptr_(new ConnectionSet)
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
		if(connection_set_ptr_.unique() == false)
		{
			connection_set_ptr_.reset(new ConnectionSet(*connection_set_ptr_));
		}
		assert(connection_set_ptr_.unique() == true);

		if(connected == true)
		{
			connection_set_ptr_->insert(connection);
		}
		else
		{
			connection_set_ptr_->erase(connection);
		}
	}
	void HandleStringMessage(const TcpConnectionPtr &connection,
	                         const string &message,
	                         TimeStamp)
	{
		ConnectionSetPtr connection_set_ptr;
		{
			MutexLockGuard lock(mutex_);
			connection_set_ptr = connection_set_ptr_;
		}
		for(ConnectionSet::iterator it = connection_set_ptr->begin();
		        it != connection_set_ptr->end();
		        ++it)
		{
			codec_.Send(*it, message);
		}
	}

	using ConnectionSet = set<TcpConnectionPtr>;
	using ConnectionSetPtr = shared_ptr<ConnectionSet>;
	TcpServer server_;
	Codec codec_;
	MutexLock mutex_;
	ConnectionSetPtr connection_set_ptr_;
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
