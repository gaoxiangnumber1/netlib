#ifndef NETLIB_NETLIB_EVENT_LOOP_THREAD_POOL_H_
#define NETLIB_NETLIB_EVENT_LOOP_THREAD_POOL_H_

#include <vector>
#include <functional>

#include <netlib/non_copyable.h>

namespace netlib
{

class EventLoop;
class EventLoopThread;

// Interface:
// Ctor
// Start
// GetNextLoop

class EventLoopThreadPool: public NonCopyable
{
public:
	using InitialTask = std::function<void(EventLoop*)>;

	explicit EventLoopThreadPool(EventLoop *main_loop,
	                             const int loop_number = 0,
	                             const InitialTask &initial_task = InitialTask());
	void Start();
	EventLoop *GetNextLoop();

private:
	EventLoop *main_loop_;
	const int loop_number_;
	std::vector<EventLoop*> loop_pool_;
	bool started_;
	int next_loop_index_;
	const InitialTask initial_task_;
};

}

#endif // NETLIB_NETLIB_EVENT_LOOP_THREAD_POOL_H_
