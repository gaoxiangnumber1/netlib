#include <netlib/event_loop_thread_pool.h>
#include <netlib/event_loop.h>
#include <netlib/event_loop_thread.h>

using netlib::EventLoop;
using netlib::EventLoopThread;
using netlib::EventLoopThreadPool;

EventLoopThreadPool::EventLoopThreadPool(EventLoop *base_loop):
	base_loop_(base_loop),
	started_(false),
	thread_number_(0),
	next_(0)
{}

EventLoopThreadPool::~EventLoopThreadPool()
{
	// Don't delete loop since it is stack variable.
}

void EventLoopThreadPool::Start()
{
	assert(started_ == false);
	base_loop_->AssertInLoopThread();

	started_ = false;
	for(int index = 0; index < thread_number_; ++index)
	{
		EventLoopThread *thread = new EventLoopThread;
		thread_pool_.push_back(thread);
		loop_pool_.push_back(thread->StartLoop());
	}
}

EventLoop *EventLoopThreadPool::GetNextLoop()
{
	base_loop_->AssertInLoopThread();

	EventLoop *next_loop = base_loop_;
	if(loop_pool_.empty() == false)
	{
		next_loop = loop_pool_[next_++];
		if(next_ >= static_cast<int>(loop_pool_.size()))
		{
			next_ = 0;
		}
	}
	return next_loop;
}
