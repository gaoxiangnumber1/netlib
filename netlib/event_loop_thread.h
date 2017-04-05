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
// Ctor -> -ThreadStartFunction
// Dtor
// StartLoop

class EventLoopThread: public NonCopyable
{
public:
	EventLoopThread(const InitialTask &initial_task = InitialTask());
	~EventLoopThread();
	EventLoop *StartLoop();

private:
	void ThreadStartFunction();

	EventLoop *loop_;
	Thread thread_;
	const InitialTask initial_task_;
	MutexLock mutex_;
	Condition condition_;
};

}

#endif // NETLIB_NETLIB_EVENT_LOOP_THREAD_H_
