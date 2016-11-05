#ifndef NETLIB_NETLIB_CALLBACK_H_
#define NETLIB_NETLIB_CALLBACK_H_

#include <functional> // function<>

namespace netlib
{

using EventCallback = std::function<void()>;
using TimerCallback = std::function<void()>;
using NewConnectionCallback = std::function<void(int, const InetAddress&)>;

}

#endif // NETLIB_NETLIB_CALLBACK_H_
