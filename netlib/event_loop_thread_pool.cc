#include <netlib/event_loop_thread_pool.h>
#include <netlib/event_loop.h>
#include <netlib/event_loop_thread.h>

using netlib::EventLoop;
using netlib::EventLoopThread;
using netlib::EventLoopThreadPool;

EventLoopThreadPool::EventLoopThreadPool(EventLoop *base_loop,
        const InitialTask &initial_task,
        const int thread_number):
	base_loop_(base_loop),
	initial_task_(initial_task),
	started_(false),
	thread_number_(thread_number),
	thread_pool_(thread_number_),
	loop_pool_(thread_number_),
	next_loop_index_(0)
{}

void EventLoopThreadPool::Start()
{
	assert(started_ == false);
	base_loop_->AssertInLoopThread();

	started_ = true;
	if(thread_number_ == 0 && initial_task_)
	{
		initial_task_(base_loop_);
	}
	for(int index = 0; index < thread_number_; ++index)
	{
		EventLoopThread *thread = new EventLoopThread(initial_task_);
		thread_pool_[index] = thread;
		loop_pool_[index] = thread->StartLoop();
	}
}

EventLoop *EventLoopThreadPool::GetNextLoop()
{
	base_loop_->AssertInLoopThread();
	assert(started_ == true);

	EventLoop *next_loop = base_loop_;
	if(thread_number_ > 0)
	{
		next_loop = loop_pool_[next_loop_index_];
		next_loop_index_ = (next_loop_index_ + 1) % thread_number_;
	}
	return next_loop;
}
