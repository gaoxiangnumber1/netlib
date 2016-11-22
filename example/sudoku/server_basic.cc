#include <stdio.h>
#include <unistd.h>

#include <utility>
#include <boost/bind.hpp>

#include <muduo/base/Atomic.h>
#include <muduo/base/Logging.h>
#include <muduo/base/Thread.h>
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

class SudokuServer
{
public:
	SudokuServer(EventLoop *loop, const InetAddress &listen_address)
		: server_(loop, listen_address, "SudokuServer"),
		  start_time_(Timestamp::now()) // Constructor.
	{
		// Set callback functions.
		server_.setConnectionCallback(bind(&SudokuServer::OnConnection, this, _1));
		server_.setMessageCallback(bind(&SudokuServer::OnMessage, this, _1, _2, _3));
	}

	void Start()
	{
		server_.start(); // Call TcpServer::start()
	}

private:
	void OnConnection(const TcpConnectionPtr &connection)
	{
		LOG_TRACE << connection->peerAddress().toIpPort() << " -> "
		          << connection->localAddress().toIpPort() << " is "
		          << (connection->connected() ? "UP" : "DOWN");
	}

	void OnMessage(const TcpConnectionPtr &connection, Buffer* buffer, Timestamp)
	{
		LOG_DEBUG << connection->name();
		size_t len = buffer->readableBytes();
		while(len >= kCells + 2) // Read request data repeatedly. 2 stands for CRLF(\r\n)
		{
			const char* crlf = buffer->findCRLF();
			if(crlf) // If found a complete request.
			{
				string request(buffer->peek(), crlf); // Copy request data.
				buffer->retrieveUntil(crlf + 2); // Retrieve data that has been read.
				len = buffer->readableBytes(); // Update buffer's length
				if(ProcessRequest(connection, request) == false) // Illegal request, close the connection.
				{
					connection->send("Bad Request!\r\n");
					connection->shutdown();
					break;
				}
			}
			else // Incomplete request.
			{
				break;
			}
		}
	}

	bool ProcessRequest(const TcpConnectionPtr &connection, const string &request)
	{
		string id;
		string puzzle;
		bool good_request = true;

		string::const_iterator colon = find(request.begin(), request.end(), ':');
		if(colon != request.end()) // If found the `id` part.
		{
			id.assign(request.begin(), colon);
			puzzle.assign(colon+1, request.end());
		}
		else
		{
			puzzle = request;
		}

		if(puzzle.size() == static_cast<size_t>(kCells)) // Request's length is legal.
		{
			LOG_DEBUG << connection->name();
			string result = SolveSudoku(puzzle); // Get answer at here.
			if(id.empty())
			{
				connection->send(result+"\r\n");
			}
			else
			{
				connection->send(id+":"+result+"\r\n");
			}
		}
		else
		{
			good_request = false;
		}
		return good_request;
	}

	TcpServer server_;
	Timestamp start_time_;
};

int main(int argc, char* argv[])
{
	LOG_INFO << "pid = " << getpid() << ", tid = " << tid();
	EventLoop loop;
	InetAddress listen_address(9981);
	SudokuServer server(&loop, listen_address);

	server.Start();

	loop.loop();
}
