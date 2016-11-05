#include <netlib/event_loop_thread.h>

#include <assert.h> // assert()

#include <functional> // bind<>

#include <netlib/event_loop.h>

using std::bind;
using netlib::EventLoopThread;
using netlib::EventLoop;

EventLoopThread::EventLoopThread():
	loop_(nullptr),
	exiting_(false),
	thread_(bind(&EventLoopThread::ThreadFunction, this)),
	mutex_(),
	condition_(mutex_)
{}

EventLoopThread::~EventLoopThread()
{
	exiting_ = true;
	loop_->Quit();
	thread_.Join();
}

EventLoop *EventLoopThread::StartLoop()
{
	assert(thread_.started() == false);
	thread_.Start();
	{
		MutexLockGuard lock(mutex_);
		while(loop_ == nullptr)
		{
			// Use condition variable to wait the creation and running of threads.
			condition_.Wait();
		}
	}
	return loop_; // Return the EventLoop object's address of the new thread.
}

void EventLoopThread::ThreadFunction()
{
	// Create object on stack.
	EventLoop loop;
	{
		MutexLockGuard lock(mutex_);
		loop_ = &loop; // Assign stack object's address to the data member.
		condition_.Notify(); // Notify the condition -> Wakeup StartLoop().
	}
	loop.Loop();
}
