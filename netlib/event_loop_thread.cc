#include <netlib/event_loop_thread.h>

#include <netlib/event_loop.h>

using std::bind;
using netlib::EventLoopThread;
using netlib::EventLoop;

EventLoopThread::EventLoopThread(const InitialTask &task):
	loop_(nullptr),
	thread_(bind(&EventLoopThread::ThreadFunction, this)),
	initial_task_(task),
	mutex_(),
	condition_(mutex_)
{}
void EventLoopThread::ThreadFunction()
{
	// Create object on stack.
	EventLoop loop;
	{
		MutexLockGuard lock(mutex_);
		loop_ = &loop; // Assign stack object's address to the data member.
		condition_.Notify(); // Notify the condition -> Wakeup StartLoop().
	}

	if(initial_task_)
	{
		initial_task_(loop_);
	}
	loop.Loop();
	loop_ = nullptr;
}

EventLoopThread::~EventLoopThread()
{
	// FIXME: Not 100% race-free, eg. ThreadFunction could be running callback_.
	if(loop_ != nullptr)
	{
		// A little chance to call destructed object, if ThreadFunction exits just now.
		// But when EventLoopThread destructs, usually programming is exiting anyway.
		loop_->Quit();
		thread_.Join();
	}
}

EventLoop *EventLoopThread::StartLoop()
{
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
