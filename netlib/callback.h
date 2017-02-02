#ifndef NETLIB_NETLIB_CALLBACK_H_
#define NETLIB_NETLIB_CALLBACK_H_

#include <functional> // function<>, bind<>
#include <memory> // shared_ptr<>
#include <netlib/time_stamp.h>

namespace netlib
{

class Buffer;
class TcpConnection;
class Connector;

using TimerCallback = std::function<void()>;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using MessageCallback = std::function<void(const TcpConnectionPtr&,
                        Buffer*,
                        TimeStamp)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr&, int)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using EventCallback = std::function<void(TimeStamp)>;

void DefaultConnectionCallback(const TcpConnectionPtr&);
void DefaultMessageCallback(const TcpConnectionPtr&, Buffer*, TimeStamp);

using NewConnectionCallback = std::function<void(int socket_fd)>;

}

#endif // NETLIB_NETLIB_CALLBACK_H_
