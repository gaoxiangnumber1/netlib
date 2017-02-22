#include "sudoku.h"

#include <algorithm>

#include <netlib/event_loop.h>
#include <netlib/socket_address.h>
#include <netlib/logging.h>
#include <netlib/tcp_server.h>

using namespace netlib;
using std::bind;
using namespace std::placeholders;
using std::string;
using std::find;

// Review: HandleMessage, ProcessRequest

// Interface:
// Ctor -> -HandleConnection -> -HandleMessage
//			-HandleMessage -> -ProcessRequest
// Start

class SudokuServer
{
public:
	SudokuServer(EventLoop *loop, const SocketAddress &listen_address):
		server_(loop, listen_address, "Sudoku_Server")
	{
		server_.set_connection_callback(bind(&SudokuServer::HandleConnection, this, _1));
		server_.set_message_callback(bind(&SudokuServer::HandleMessage, this, _1, _2, _3));
	}

	void Start()
	{
		server_.Start();
	}

private:
	void HandleConnection(const TcpConnectionPtr &connection)
	{
		LOG_TRACE("%s -> %s is %s",
		          connection->peer_address().ToIpPortString().c_str(),
		          connection->local_address().ToIpPortString().c_str(),
		          (connection->Connected() ? "UP" : "DOWN"));
	}
	void HandleMessage(const TcpConnectionPtr &connection, Buffer *buffer, TimeStamp)
	{
		LOG_DEBUG("%s", connection->name().c_str());
		int length = buffer->ReadableByte();
		while(length >= kCellNumber + 2) // 2 stands for CRLF(\r\n)
		{
			bool good_request;
			const char *crlf = buffer->FindCRLF();
			if(crlf != nullptr) // If found a complete request.
			{
				string request(buffer->ReadableBegin(), crlf);
				buffer->RetrieveUntil(crlf + 2); // Move over "\r\n"
				length = buffer->ReadableByte();
				good_request = ProcessRequest(connection, request);
			}
			if(crlf == nullptr || good_request == false)
			{
				connection->Send("Bad Request!\r\n");
				connection->Shutdown();
				break;
			}
		}
	}
	bool ProcessRequest(const TcpConnectionPtr &connection, const string &request)
	{
		string id, puzzle;
		string::const_iterator colon = find(request.begin(), request.end(), ':');
		if(colon != request.end())
		{
			id.assign(request.begin(), colon);
			puzzle.assign(colon + 1, request.end());
		}
		else
		{
			puzzle = request;
		}

		bool good_request = true;
		if(static_cast<int>(puzzle.size()) == kCellNumber)
		{
			TimeStamp start(TimeStamp::Now());
			string result = SolveSudoku(puzzle);
			LOG_INFO("%f", TimeDifferenceInSecond(TimeStamp::Now(), start));
			if(id.empty() == false)
			{
				connection->Send(id + ":");
			}
			connection->Send(result + "\r\n");
		}
		else
		{
			good_request = false;
		}
		return good_request;
	}

	TcpServer server_;
};

int main(int argc, char* argv[])
{
	EventLoop loop;
	SocketAddress listen_address(7188);
	SudokuServer server(&loop, listen_address);
	server.Start();
	loop.Loop();
}
