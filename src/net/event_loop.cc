#include "event_loop.h"

__thread EventLoop *g_loop_in_this_thread = nullptr;

EventLoop::EventLoop(): looping_(false), thread_id_(CurrentThread::tid())
{
	LOG_TRACE << "EventLoop created " << this << " in thread " << thread_id_;
	// One loop per thread means that every thread can only have one EventLoop object,
	// check it by following test. If this thread already has other EventLoop object,
	// abort this thread.
	if(g_loop_in_this_thread != nullptr)
	{
		LOG_FATAL << "Another EventLoop " << g_loop_in_this_thread <<
		          "exists in this thread " << thread_id_;
	}
	else
	{
		g_loop_in_this_thread = this;
	}
}

EventLoop::~EventLoop()
{
	assert(looping_ == false);
	g_loop_in_this_thread = nullptr;
}

void EventLoop::Loop()
{
	assert(looping_ == false); // Not in looping.
	AssertInLoopThread();
	looping_ = true;

	::poll(NULL, 0, 5 * 1000);

	LOG_TRACE << "EventLoop " << this << " Stop looping.";
	looping_ = false;
}

// Return the object that hold by current thread, nullptr means current thread
// is not the IO thread.
EventLoop *GetEventLoopOfCurrentThread() // static member function.
{
	return g_loop_in_this_thread;
}

void EventLoop::AbortNotInLoopThread()
{
	LOG_FATAL << "EventLoop::AbortNotInLoopThread - EventLoop " << this
	          << " was created in thread_id = " << thread_id_
	          << ", current thread id = " <<  CurrentThread::tid();
}
