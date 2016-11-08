#ifndef NETLIB_NETLIB_EVENT_LOOP_THREAD_POOL_H_
#define NETLIB_NETLIB_EVENT_LOOP_THREAD_POOL_H_

#include <netlib/non_copyable.h>
#include <netlib/logging.h>
#include <vector>

namespace netlib
{

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool: public NonCopyable
{
public:
	EventLoopThreadPool(EventLoop *base_loop);
	~EventLoopThreadPool();
	void set_thread_number(int thread_number)
	{
		thread_number_ = thread_number;
		LOG_INFO("thread_number_ = %d", thread_number_);
	}
	void Start();
	EventLoop *GetNextLoop();

private:
	EventLoop *base_loop_;
	bool started_;
	int thread_number_;
	int next_; // Always in loop thread.
	std::vector<EventLoopThread*> thread_pool_;
	std::vector<EventLoop*> loop_pool_;
};

}

#endif // NETLIB_NETLIB_EVENT_LOOP_THREAD_POOL_H_
