#include <netlib/event_loop_thread.h>

#include <netlib/event_loop.h>

using std::bind;
using netlib::EventLoopThread;
using netlib::EventLoop;

EventLoopThread::EventLoopThread(const InitialTask &initial_task):
	loop_(nullptr),
	thread_(bind(&EventLoopThread::ThreadFunction, this)),
	initial_task_(initial_task),
	mutex_(),
	condition_(mutex_)
{}
void EventLoopThread::ThreadFunction()
{
	EventLoop loop;
	{
		MutexLockGuard lock(mutex_);
		loop_ = &loop;
		condition_.Signal();
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
	if(loop_ != nullptr)
	{
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
			condition_.Wait();
		}
	}
	return loop_;
}
