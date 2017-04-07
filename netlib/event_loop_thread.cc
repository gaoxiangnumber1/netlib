#include <netlib/event_loop_thread.h>

#include <netlib/event_loop.h>

using std::bind;
using netlib::EventLoopThread;
using netlib::EventLoop;

EventLoopThread::EventLoopThread():
	loop_(nullptr),
	thread_(bind(&EventLoopThread::ThreadMainFunction, this)),
	mutex_(),
	condition_(mutex_)
{}
void EventLoopThread::ThreadMainFunction()
{
	EventLoop loop;
	{
		MutexLockGuard lock(mutex_);
		loop_ = &loop;
		condition_.Signal();
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
