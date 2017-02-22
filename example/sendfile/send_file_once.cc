#include <stdio.h>

#include <netlib/logging.h>
#include <netlib/event_loop.h>
#include <netlib/tcp_server.h>

// Review: HandleConnection#set_cb, ReadFile#fclose

using namespace netlib;
using std::string;

const char *g_file = nullptr;
const int kBufferSize = 1024 * 1024;

// Three types of buffer: unbuffered, block buffered, and line buffered.
// When an output stream is unbuffered, information appears on the destination file
// or terminal as soon as written; when it is block buffered, many characters are
// saved up and written as a block; when it is line buffered, characters are saved up
// until a newline is output or input is read from any stream attached to a terminal
// device (typically stdin). fflush(3) may be used to force the block out early.
// Normally all files are block buffered.
// When the first I/O operation occurs on a file, malloc(3) is called,
// and a buffer is obtained. If a stream refers to a terminal, it is line buffered.
// stderr is unbuffered by default.
// int setvbuf(FILE *stream, char *buf, int mode, size_t size);
// setvbuf() may be used on any open stream to change its buffer.
// mode must be one of the following three macros:
// _IONBF unbuffered; _IOLBF line buffered; _IOFBF fully buffered
// Except for unbuffered files, buf should point to a buffer at least size bytes long;
// this buffer will be used instead of the current buffer. If buf is NULL, only the
// mode is affected; a new buffer will be allocated on the next read or write operation.
// setvbuf() may be used only after opening a stream and before any other operations
// have been performed on it. The other three calls are aliases for calls to setvbuf().
// void setbuf(FILE *stream, char *buf);
// setbuf() is equivalent to: setvbuf(stream, buf, buf ? _IOFBF : _IONBF, BUFSIZ);
// void setbuffer(FILE *stream, char *buf, size_t size);
// setbuffer() is the same, except that the size of the buffer is up to the caller,
// rather than being determined by the default BUFSIZ.
// void setlinebuf(FILE *stream);
// setlinebuf() is equivalent to the call: setvbuf(stream, NULL, _IOLBF, 0);
// setvbuf() returns 0 on success, nonzero on failure and errno is set (mode is invalid
// or the request cannot be honored). The other functions do not return a value.

string ReadFile(const char *file)
{
	string file_content;
	FILE *fp = ::fopen(file, "rb");
	if(fp != nullptr)
	{
		char io_buffer[kBufferSize];
		::setbuffer(fp, io_buffer, sizeof io_buffer);

		char buffer[kBufferSize];
		int read_byte = 0;
		while((read_byte = static_cast<int>(::fread(buffer, 1, sizeof buffer, fp))) > 0)
		{
			file_content.append(buffer, read_byte);
		}
		::fclose(fp);
	}
	return file_content;
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
		connection->set_high_water_mark_callback(HandleHighWaterMark, 64 * 1024);
		string file_content = ReadFile(g_file);
		connection->Send(file_content);
		connection->Shutdown();
		LOG_INFO("FileServer - Done.");
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
	server.Start();
	loop.Loop();
}
