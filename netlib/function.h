#ifndef NETLIB_NETLIB_FUNCTION_H_
#define NETLIB_NETLIB_FUNCTION_H_

#include <functional> // function<>, bind<>
#include <memory> // shared_ptr<>
#include <netlib/time_stamp.h>

namespace netlib
{
// TODO: Move each std::function to it own class.
class EventLoop;
class Buffer;
class TcpConnection;
class Connector;

using InitialTask = std::function<void(EventLoop*)>;

using TimerCallback = std::function<void()>;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using MessageCallback = std::function<void(const TcpConnectionPtr&,
                        Buffer*,
                        const TimeStamp&)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr&, int)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using EventCallback = std::function<void(const TimeStamp&)>;
using TaskCallback = std::function<void()>;

}

#endif // NETLIB_NETLIB_FUNCTION_H_
