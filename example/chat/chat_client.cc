#include "codec.h"

#include <stdio.h>
#include <unistd.h>

#include <netlib/event_loop_thread.h>
#include <netlib/logging.h>
#include <netlib/mutex.h>
#include <netlib/tcp_client.h>

using namespace netlib;
using namespace std::placeholders;
using std::bind;
using std::string;

const int kOneKilobyte = 1024;

class ChatClient: public NonCopyable
{
public:
	ChatClient(EventLoop *loop, const SocketAddress &server_address):
		client_(loop, server_address, "ChatClient"),
		codec_(bind(&ChatClient::HandleStringMessage, this, _1, _2, _3))
	{
		client_.set_connection_callback(bind(&ChatClient::HandleConnection, this, _1));
		client_.set_message_callback(bind(&Codec::HandleMessage, &codec_, _1, _2, _3));
		client_.EnableRetry();
	}

	void Connect()
	{
		client_.Connect();
	}
	void Disconnect()
	{
		client_.Disconnect();
	}

	void Write(const string &message)
	{
		MutexLockGuard lock(mutex_);
		if(connection_)
		{
			codec_.Send(connection_, message);
		}
	}

private:
	void HandleConnection(const TcpConnectionPtr &connection)
	{
		bool connected = connection->Connected();
		LOG_INFO("ChatClient - %s -> %s is %s",
		         connection->local_address().ToIpPortString().c_str(),
		         connection->peer_address().ToIpPortString().c_str(),
		         (connected ? "UP" : "DOWN"));
		MutexLockGuard lock(mutex_);
		if(connected == true)
		{
			connection_ = connection;
		}
		else
		{
			connection_.reset();
		}
	}
	void HandleStringMessage(const TcpConnectionPtr &connection,
	                         const string &message,
	                         TimeStamp receive_time)
	{
		printf("%s: %s", receive_time.ToFormattedTimeString().c_str(), message.c_str());
	}

	TcpClient client_;
	Codec codec_;
	MutexLock mutex_;
	TcpConnectionPtr connection_;
};

int main(int argc, char **argv)
{
	if(argc != 3)
	{
		LOG_FATAL("Usage: %s server_ip server_port", argv[0]);
	}

	EventLoopThread loop_thread;
	SocketAddress server_address(argv[1], atoi(argv[2]));
	ChatClient chat_client(loop_thread.StartLoop(), server_address);
	chat_client.Connect();
	char buffer[kOneKilobyte];
	while(fgets(buffer, sizeof buffer, stdin) != nullptr)
	{
		chat_client.Write(buffer);
	}
	chat_client.Disconnect();
	sleep(1);
	// Wait for disconnect, then safe to destruct Client.
	// Otherwise mutex_ is used after dtor.
}
