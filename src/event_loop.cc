#include <event_loop.h>

#include <assert.h> // assert()
#include <poll.h> // poll()

#include <logging.h>
#include <thread.h>

using netlib::Thread;
using netlib::EventLoop;

// Every thread has its own instance of __thread variable.
__thread EventLoop *t_loop_in_this_thread = nullptr;

EventLoop::EventLoop():
	looping_(false),
	quit_(false),
	thread_id_(Thread::ThreadId()),
	poller_(new Poller(this))
{
	LOG_INFO("EventLoop created %p in thread %d", this, thread_id_);
	// One loop per thread means that every thread can have only one EventLoop object.
	// If this thread already has another EventLoop object, abort this thread.
	if(t_loop_in_this_thread != nullptr)
	{
		LOG_FATAL("Another EventLoop %p exists in this thread %d",
		          t_loop_in_this_thread, thread_id_);
	}
	else
	{
		// The thread that creates EventLoop object is the IO thread,
		// whose main function is running EventLoop::Loop().
		t_loop_in_this_thread = this;
	}
}

EventLoop::~EventLoop()
{
	assert(looping_ == false);
	t_loop_in_this_thread = nullptr;
}

void EventLoop::Loop()
{
	assert(looping_ == false); // Not in looping.
	AssertInLoopThread();
	looping_ = true;
	quit_ = false;

	while(quit_ == false)
	{
		active_channel_.clear();
		poller_->Poll(kPollTimeout, &active_channel_);
		for(ChannelVector::iterator it = active_channel_.begin();
		        it != active_channel_.end(); ++it)
		{
			(*it)->HandleEvent();
		}
	}

	LOG_INFO("EventLoop %p Stop looping.", this);
	looping_ = false;
}

void AssertInLoopThread()
{
	if(thread_id_ != Thread::ThreadId()) // Must run Loop in IO thread.
	{
		LOG_FATAL("EventLoop %p was created in tid = %d, current tid = %d",
		          this, thread_id_, Thread::ThreadId());
	}
}

void Quit()
{
	quit_ = true;
}

void UpdateChannel(Channel *channel)
{
	assert(channel->loop() == this);
	AssertInLoopThread();
	poller_->UpdateChannel(channel);
}

// Return the object that hold by current thread.
// nullptr means current thread is not the IO thread.
EventLoop *GetEventLoopOfCurrentThread() // static member function.
{
	return t_loop_in_this_thread;
}
