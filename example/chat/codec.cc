#include "codec.h"

#include <endian.h> // htobe32()

#include <string>

#include <netlib/buffer.h>
#include <netlib/tcp_connection.h>
#include <netlib/time_stamp.h>
#include <netlib/logging.h>

using namespace netlib;
using std::string;

Codec::Codec(const StringMessageCallback &callback):
	string_message_callback_(callback)
{}

void Codec::Send(const TcpConnectionPtr &connection, const string &message)
{
	Buffer buffer;
	int32_t length = static_cast<int32_t>(message.size());
	int32_t length_be32 = ::htobe32(length);
	buffer.Prepend(&length_be32, static_cast<int>(sizeof length_be32));
	buffer.Append(message.data(), length);
	connection->Send(&buffer);
}

void Codec::HandleMessage(const TcpConnectionPtr &connection,
                          Buffer *buffer,
                          TimeStamp receive_time)
{
	while(buffer->ReadableByte() >= kHeaderLength)
	{
		int32_t length = buffer->PeekInt32();
		if(length < kMinMessageLength || length > kMaxMessageLength)
		{
			LOG_ERROR("Invalid length = %d", length);
			connection->Shutdown();
			break;
		}
		else if(buffer->ReadableByte() >= kHeaderLength + length)
		{
			buffer->Retrieve(kHeaderLength);
			string message(buffer->ReadableBegin(), length);
			string_message_callback_(connection, message, receive_time);
			buffer->Retrieve(length);
		}
		else
		{
			break;
		}
	}
}
