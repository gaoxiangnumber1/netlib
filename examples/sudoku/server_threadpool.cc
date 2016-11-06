#include <stdio.h>
#include <unistd.h>

#include <utility>
#include <boost/bind.hpp>

#include <muduo/base/Atomic.h>
#include <muduo/base/Logging.h>
#include <muduo/base/Thread.h>
#include <muduo/base/ThreadPool.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpServer.h>

#include "sudoku.h"

using muduo::net::EventLoop;
using muduo::net::InetAddress;
using muduo::Timestamp;
using muduo::net::TcpServer;
using boost::bind;
using muduo::net::TcpConnectionPtr;
using muduo::net::Buffer;
using muduo::string;
using muduo::CurrentThread::tid;
using muduo::ThreadPool;

class SudokuServer
{
public:
	SudokuServer(EventLoop *loop, const InetAddress &listen_address, int thread_number)
		: server_(loop, listen_address, "SudokuServer"),
		  thread_number_(thread_number),
		  start_time_(Timestamp::now())
	{
		server_.setConnectionCallback(bind(&SudokuServer::OnConnection, this, _1));
		server_.setMessageCallback(bind(&SudokuServer::OnMessage, this, _1, _2, _3));
	}

	void Start()
	{
		LOG_INFO << "starting " << thread_number_ << " threads.";
		thread_pool_.start(thread_number_);
		server_.start();
	}

private:
	void OnConnection(const TcpConnectionPtr &connection)
	{
		LOG_TRACE << connection->peerAddress().toIpPort() << " -> "
		          << connection->localAddress().toIpPort() << " is "
		          << (connection->connected() ? "UP" : "DOWN");
	}

	void OnMessage(const TcpConnectionPtr &connection, Buffer *buffer, Timestamp)
	{
		LOG_DEBUG << connection->name();
		size_t len = buffer->readableBytes();
		while(len >= kCells + 2)
		{
			const char *crlf = buffer->findCRLF();
			if(crlf)
			{
				string request(buffer->peek(), crlf);
				buffer->retrieveUntil(crlf + 2);
				len = buffer->readableBytes();
				if (!processRequest(connection, request))
				{
					connection->send("Bad Request!\r\n");
					connection->shutdown();
					break;
				}
			}
			else
			{
				break;
			}
		}
	}

	bool ProcessRequest(const TcpConnectionPtr &connection, const string &request)
	{
		string id;
		string puzzle;
		bool goodRequest = true;

		string::const_iterator colon = find(request.begin(), request.end(), ':');
		if (colon != request.end())
		{
			id.assign(request.begin(), colon);
			puzzle.assign(colon+1, request.end());
		}
		else
		{
			puzzle = request;
		}

		if (puzzle.size() == static_cast<size_t>(kCells))
		{
			thread_pool_.run(bind(&solve, connection, puzzle, id));
		}
		else
		{
			goodRequest = false;
		}
		return goodRequest;
	}

	static void solve(const TcpConnectionPtr &connection,
	                  const string &puzzle,
	                  const string &id)
	{
		LOG_DEBUG << connection->name();
		string result = SolveSudoku(puzzle);
		if (id.empty())
		{
			connection->send(result+"\r\n");
		}
		else
		{
			connection->send(id+":"+result+"\r\n");
		}
	}

	TcpServer server_;
	ThreadPool thread_pool_;
	int thread_number_;
	Timestamp start_time_;
};

int main(int argc, char *argv[])
{
	LOG_INFO << "pid = " << getpid() << ", tid = " << tid();
	int thread_number = 0;
	if(argc > 1)
	{
		thread_number = atoi(argv[1]);
	}
	EventLoop loop;
	InetAddress listen_address(7188);
	SudokuServer server(&loop, listen_address, thread_number);
	server.Start();
	loop.loop();
}

