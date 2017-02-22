#include <stdio.h>

#include <netlib/logging.h>
#include <netlib/event_loop.h>
#include <netlib/tcp_server.h>

// Review: HandleConnection, HandleWriteComplete

using namespace netlib;

const char *g_file = nullptr;
const int kBufferSize = 64 * 1024; // 64KB

void HandleWriteComplete(const TcpConnectionPtr &connection)
{
	FILE *fp = static_cast<FILE*>(connection->context());
	char buffer[kBufferSize];
	int read_byte = static_cast<int>(::fread(buffer, 1, sizeof buffer, fp));
	if(read_byte > 0)
	{
		LOG_INFO("Enter HandleWriteComplete: Resend");
		connection->Send(buffer, read_byte);
	}
	else
	{
		::fclose(fp);
		fp = nullptr;
		connection->set_context(nullptr);
		connection->Shutdown();
		LOG_INFO("FileServer - Done.");
	}
}
void HandleHighWaterMark(const TcpConnectionPtr &connection, int length)
{
	LOG_INFO("HighWaterMark = %d", length);
}
void HandleConnection(const TcpConnectionPtr &connection)
{
	bool connected = connection->Connected();
	LOG_INFO("FileServer - %s -> %s is %s",
	         connection->peer_address().ToIpPortString().c_str(),
	         connection->local_address().ToIpPortString().c_str(),
	         (connected ? "UP" : "DOWN"));
	if(connected == true)
	{
		LOG_INFO("FileServer - Sending file %s to %s",
		         g_file,
		         connection->peer_address().ToIpPortString().c_str());
		connection->set_high_water_mark_callback(HandleHighWaterMark, kBufferSize + 1);

		FILE *fp = ::fopen(g_file, "rb");
		if(fp != nullptr)
		{
			connection->set_context(fp);
			char buffer[kBufferSize];
			int read_byte = static_cast<int>(::fread(buffer, 1, sizeof buffer, fp));
			connection->Send(buffer, read_byte);
		}
		else
		{
			connection->Shutdown();
			LOG_FATAL("FileServer - No such file.");
		}
	}
	else
	{
		// Must close FILE stream, otherwise resource escape.
		if(connection->context() != nullptr)
		{
			::fclose(static_cast<FILE*>(connection->context()));
		}
	}
}

int main(int argc, char **argv)
{
	if(argc == 1)
	{
		LOG_FATAL("Usage: %s file_for_sending", argv[0]);
	}

	g_file = argv[1];
	EventLoop loop;
	SocketAddress listen_address(7188);
	TcpServer server(&loop, listen_address, "FileServer");
	server.set_connection_callback(HandleConnection);
	server.set_write_complete_callback(HandleWriteComplete);
	server.Start();
	loop.Loop();
}
