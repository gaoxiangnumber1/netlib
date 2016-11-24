#ifndef NETLIB_NETLIB_EVENT_LOOP_THREAD_POOL_H_
#define NETLIB_NETLIB_EVENT_LOOP_THREAD_POOL_H_

#include <vector>
#include <functional>

#include <netlib/non_copyable.h>

namespace netlib
{

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool: public NonCopyable
{
public:
	using InitialTask = std::function<void(EventLoop*)>;

	explicit EventLoopThreadPool(EventLoop *base_loop,
	                             const int thread_number,
	                             const InitialTask &initial_task = InitialTask());
	~EventLoopThreadPool();
	void Start();
	EventLoop *GetNextLoop();

private:
	EventLoop *base_loop_;
	const int thread_number_;
	InitialTask initial_task_;
	bool started_;
	int next_loop_index_; // Always in loop thread.
	std::vector<EventLoopThread*> thread_pool_;
	std::vector<EventLoop*> loop_pool_;
};

}

#endif // NETLIB_NETLIB_EVENT_LOOP_THREAD_POOL_H_
