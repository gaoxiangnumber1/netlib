#ifndef NETLIB_NETLIB_CALLBACK_H_
#define NETLIB_NETLIB_CALLBACK_H_

#include <functional> // function<>, bind<>

namespace netlib
{
class SocketAddress;
class TcpConnection;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using ConnectionCallback = std::function<void(const TcpConnection&)>;
using MessageCallback = std::function<void(const TcpConnectionPtr&, const char*, int)>;
using EventCallback = std::function<void()>;
using TimerCallback = std::function<void()>;

}

#endif // NETLIB_NETLIB_CALLBACK_H_
