#ifndef NETLIB_NETLIB_EVENT_LOOP_THREAD_H_
#define NETLIB_NETLIB_EVENT_LOOP_THREAD_H_

#include <netlib/condition.h>
#include <netlib/mutex.h>
#include <netlib/non_copyable.h>
#include <netlib/thread.h>

#include <netlib/function.h>

namespace netlib
{

class EventLoop;

// Interface:
// Ctor -> -ThreadMainFunction
// Dtor
// StartLoop

class EventLoopThread: public NonCopyable
{
public:
	EventLoopThread();
	~EventLoopThread();
	EventLoop *StartLoop();

private:
	void ThreadMainFunction();

	EventLoop *loop_;
	Thread thread_;
	MutexLock mutex_;
	Condition condition_;
};

}

#endif // NETLIB_NETLIB_EVENT_LOOP_THREAD_H_
