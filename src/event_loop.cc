#include <event_loop.h>

#include <assert.h> // assert()
#include <poll.h> // poll()

#include <channel.h>
#include <logging.h>
#include <poller.h>
#include <thread.h>
#include <timer_queue.h>

using netlib::EventLoop;
using netlib::Thread;
using netlib::TimerId;
using netlib::TimerCallback;
using netlib::TimerQueue;
using netlib::TimeStamp;

// Every thread has its own instance of __thread variable.
__thread EventLoop *t_loop_in_this_thread = nullptr;
const int kPollTimeout = 2 * 1000;

EventLoop::EventLoop():
	looping_(false),
	quit_(false),
	thread_id_(Thread::ThreadId()),
	poller_(new Poller(this)), // Set poller_'s owner_loop_ to this EventLoop.
	timer_queue_(new TimerQueue(this))
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

// Return the object that hold by current thread.
// nullptr means current thread is not the IO thread.
EventLoop *EventLoop::GetEventLoopOfCurrentThread() // static member function.
{
	return t_loop_in_this_thread;
}

// We can set quit_ to be true by calling set_quit(true) to terminate loop. But the actual
// quit happen when EventLoop::Loop() check `while(quit_ == false)`. If set_quit(true)
// happens in other threads(not in current IO thread), the delay can be a few seconds.
// set_quit(true) only set a flag, not generate an interrupt or a signal. So, when Loop
// is blocking in some calls, set_quit(true) can't take effects until Loop is unblocking.
void EventLoop::set_quit(bool quit)
{
	quit_ = quit;
}

void EventLoop::AssertInLoopThread()
{
	if(thread_id_ != Thread::ThreadId()) // Must run Loop in IO thread.
	{
		LOG_FATAL("EventLoop %p was created in tid = %d, current tid = %d",
		          this, thread_id_, Thread::ThreadId());
	}
}

void EventLoop::Loop()
{
	// Check invariants by assert().
	assert(looping_ == false); // Not in looping.
	AssertInLoopThread(); // Must run Loop in IO thread.

	looping_ = true;
	quit_ = false;
	// Loop forever unless quit_ is set to `true` by current IO thread or other thread.
	while(quit_ == false)
	{
		active_channel_.clear(); // Clear old active channel vector.
		// Invoke ::poll() to get the number of active IO events and
		// invoke FillActiveChannel to fill active_channel_.
		poll_return_time_ = poller_->Poll(kPollTimeout, &active_channel_);
		LOG_INFO("Poll Return.");
		// Now, each channel in active_channel_ has IO events happened.
		for(ChannelVector::iterator it = active_channel_.begin();
		        it != active_channel_.end(); ++it)
		{
			(*it)->HandleEvent();
		}
	}

	LOG_INFO("EventLoop %p Stop looping.", this);
	looping_ = false;
}

// Runs callback at `time_stamp`.
TimerId EventLoop::RunAt(const TimerCallback &callback,
                         const TimeStamp &time_stamp)
{
	// TimerId AddTimer(const TimerCallback &callback,
	//                  TimeStamp expired_time,
	//                  double interval);
	return timer_queue_->AddTimer(callback, time_stamp, 0.0);
}

// Run callback after `delay` seconds.
TimerId EventLoop::RunAfter(const TimerCallback &callback, double delay)
{
	TimeStamp time_stamp(AddTime(TimeStamp::Now(), delay));
	return RunAt(callback, time_stamp);
}

// Run callback every `interval` seconds.
TimerId EventLoop::RunEvery(const TimerCallback &callback, double interval)
{
	TimeStamp time_stamp(AddTime(TimeStamp::Now(), interval));
	return timer_queue_->AddTimer(callback, time_stamp, interval);
}

void EventLoop::UpdateChannel(Channel *channel)
{
	// Only can update channel that this EventLoop owns.
	assert(channel->owner_loop() == this);
	AssertInLoopThread();
	poller_->UpdateChannel(channel);
}

void QueueInLoop(const Functor &callback)
{
	{
		MutexLockGuard lock(mutex_);
		pending_functor_.push_back(callback);
	}

	if(IsLoopInThread() == false || calling_pending_functor_ == true)
	{
		Wakeup();
	}
}
