#ifndef NETLIB_NETLIB_EVENT_LOOP_THREAD_H_
#define NETLIB_NETLIB_EVENT_LOOP_THREAD_H_

#include <netlib/condition.h>
#include <netlib/mutex.h>
#include <netlib/non_copyable.h>
#include <netlib/thread.h>

namespace netlib
{

class EventLoop;

class EventLoopThread: public NonCopyable
{
public:
	EventLoopThread();
	~EventLoopThread();
	EventLoop *StartLoop();

private:
	void ThreadFunction();

	EventLoop *loop_;
	bool exiting_;
	Thread thread_;
	MutexLock mutex_;
	Condition condition_;
};

}

#endif // NETLIB_NETLIB_EVENT_LOOP_THREAD_H_
