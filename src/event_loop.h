#ifndef NETLIB_SRC_NET_EVENTLOOP_H_
#define NETLIB_SRC_NET_EVENTLOOP_H_

#include <non_copyable.h>

namespace netlib
{
namespace net
{
class EventLoop: public netlib::NonCopyable
{
public:
	EventLoop();
	~EventLoop();

	void Loop();
	bool IsInLoopThread() const
	{
		return thread_id_ == CurrentThread::tid();
	}
	void AssertInLoopThread()
	{
		if(!IsInLoopThread())
		{
			AbortNotInLoopThread();
		}
	}

	// Return the object that hold by current thread.
	static EventLoop *GetEventLoopOfCurrentThread();

private:
	void AbortNotInLoopThread();

	bool looping_; // Atomic
	const pid_t thread_id_;
};
}
}
#endif // NETLIB_SRC_NET_EVENTLOOP_H_
