#include <netlib/event_loop_thread_pool.h>
#include <netlib/event_loop.h>
#include <netlib/event_loop_thread.h>

using netlib::EventLoop;
using netlib::EventLoopThread;
using netlib::EventLoopThreadPool;

EventLoopThreadPool::EventLoopThreadPool(EventLoop *main_loop,
        const int loop_number):
	main_loop_(main_loop),
	loop_number_(loop_number),
	loop_pool_(loop_number_),
	started_(false),
	next_loop_index_(0)
{}

void EventLoopThreadPool::Start()
{
	assert(started_ == false);
	main_loop_->AssertInLoopThread();

	started_ = true;
	for(int index = 0; index < loop_number_; ++index)
	{
		EventLoopThread *thread = new EventLoopThread();
		loop_pool_[index] = thread->StartLoop();
	}
}

EventLoop *EventLoopThreadPool::GetNextLoop()
{
	main_loop_->AssertInLoopThread();
	assert(started_ == true);

	EventLoop *next_loop = main_loop_;
	if(loop_number_ > 0)
	{
		next_loop = loop_pool_[next_loop_index_];
		next_loop_index_ = (next_loop_index_ + 1) % loop_number_;
	}
	return next_loop;
}
