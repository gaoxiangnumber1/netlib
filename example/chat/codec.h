#ifndef NETLIB_EXAMPLE_CHAT_CODEC_H_
#define NETLIB_EXAMPLE_CHAT_CODEC_H_

#include <netlib/non_copyable.h>
#include <netlib/callback.h>

class Codec: public netlib::NonCopyable
{
public:
	using StringMessageCallback = std::function<void(const netlib::TcpConnectionPtr&,
	                              const std::string&,
	                              netlib::TimeStamp)>;

	explicit Codec(const StringMessageCallback&);
	void Send(const netlib::TcpConnectionPtr&, const std::string&);
	void HandleMessage(const netlib::TcpConnectionPtr&, netlib::Buffer*, netlib::TimeStamp);

private:
	StringMessageCallback string_message_callback_;
	static const int kHeaderLength = 4;
	static const int kMinMessageLength = 0;
	static const int kMaxMessageLength = 64 * 1024; // 64KB
};

#endif // NETLIB_EXAMPLE_CHAT_CODEC_H_
