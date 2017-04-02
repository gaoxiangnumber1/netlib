#include <netlib/timer_queue.h>

#include <assert.h>
#include <strings.h> // bzero()
#include <sys/timerfd.h> // timerfd_*
#include <unistd.h> // close(), read()

#include <algorithm>

#include <netlib/channel.h>
#include <netlib/event_loop.h>
#include <netlib/logging.h>
#include <netlib/timer.h>
#include <netlib/timer_id.h>

using std::bind;
using std::pair;
using netlib::Channel;
using netlib::EventLoop;
using netlib::TimerId;
using netlib::TimerQueue;

TimerQueue::TimerQueue(EventLoop *owner_loop):
	owner_loop_(owner_loop),
	timer_fd_(CreateTimerFd()),
	timer_fd_channel_(owner_loop_, timer_fd_)
{
	// The function signature can be different. We can choose not to use the original
	// signature's arguments, if so, the parameter(s) supplied were ignored.
	timer_fd_channel_.set_event_callback(Channel::READ_CALLBACK,
	                                     bind(&TimerQueue::HandleRead, this));
	timer_fd_channel_.set_requested_event(Channel::READ_EVENT);
}
int TimerQueue::CreateTimerFd()
{
	int timer_fd = ::timerfd_create(CLOCK_MONOTONIC,
	                                TFD_NONBLOCK | TFD_CLOEXEC);
	if(timer_fd < 0)
	{
		LOG_FATAL("timerfd_create():: FATAL");
	}
	return timer_fd;
}
void TimerQueue::HandleRead()
{
	owner_loop_->AssertInLoopThread();
	// 1. Get expired time and read timer fd.
	TimeStamp expired_time(TimeStamp::Now());
	ReadTimerFd(expired_time);
	// 2.	Get expired timers and Run their callback.
	GetAndRemoveExpiredTimer(expired_time);
	for(TimerVector::iterator it = expired_timer_vector_.begin();
	        it != expired_timer_vector_.end();
	        ++it)
	{
		(*it)->Run();
	}
	// 3. Refresh state for next expiration.
	Refresh(expired_time);
}

void TimerQueue::ReadTimerFd(const TimeStamp &time_stamp)
{
	uint64_t expired_number;
	int read_byte =
	    static_cast<int>(::read(timer_fd_, &expired_number, sizeof expired_number));
	if(read_byte != sizeof expired_number)
	{
		LOG_ERROR("TimerQueue::ReadTimerFd read %d bytes instead of 8.", read_byte);
	}
	LOG_TRACE("expired number = %d, time = %s",
	          static_cast<int>(expired_number),
	          time_stamp.ToFormattedTimeString().c_str());
}
void TimerQueue::GetAndRemoveExpiredTimer(const TimeStamp &expired_time)
{
	// 1. Find the first not expired timer by sentry.
	ExpirationTimerPair sentry(expired_time, reinterpret_cast<Timer*>(UINTPTR_MAX));
	ExpirationTimerPairSet::iterator first_not_expired =
	    active_timer_set_.lower_bound(sentry);
	assert(first_not_expired == active_timer_set_.end() ||
	       expired_time < first_not_expired->first);
	// 2. Copy expired timers to expired timer vector.
	for(ExpirationTimerPairSet::iterator it = active_timer_set_.begin();
	        it != first_not_expired;
	        ++it)
	{
		expired_timer_vector_.push_back(it->second);
	}
	// 3.	Remove expired timers in active timer set.
	active_timer_set_.erase(active_timer_set_.begin(), first_not_expired);
}
void TimerQueue::Refresh(const TimeStamp &expired_time)
{
	// 1. Expired timer: (1) Restart if repeatable and not be canceled. (2) Deleted otherwise.
	for(TimerVector::iterator it = expired_timer_vector_.begin();
	        it != expired_timer_vector_.end();
	        ++it)
	{
		if((*it)->repeat() == true &&
		        canceling_timer_set_.find((*it)->sequence()) == canceling_timer_set_.end())
		{
			(*it)->Restart(expired_time);
			InsertIntoActiveTimerSet(*it);
		}
		else
		{
			delete *it;
		}
	}
	// 2. Set next expired time.
	if(active_timer_set_.empty() == false)
	{
		SetExpiredTime(active_timer_set_.begin()->first);
	}
	// 3. Clear expired and canceled timers.
	expired_timer_vector_.clear();
	canceling_timer_set_.clear();
}

bool TimerQueue::InsertIntoActiveTimerSet(Timer *timer)
{
	owner_loop_->AssertInLoopThread();

	TimeStamp expired_time(timer->expired_time());
	active_timer_set_.insert(ExpirationTimerPair(expired_time, timer));
	bool is_first_expired = false;
	if(expired_time == active_timer_set_.begin()->first)
	{
		is_first_expired = true;
	}
	return is_first_expired;
	// Don't `SetExpiredTime()` if is_first_expired is true, leave update decision to caller.
	// Reduce number of calling `timerfd_settime()` in Refresh().
}
void TimerQueue::SetExpiredTime(const TimeStamp &expired_time)// Absolute expiration.
{
	int64_t diff_microsecond =
	    expired_time.microsecond() - TimeStamp::Now().microsecond();
	if(diff_microsecond < 1000) // Round up and save system call.
	{
		diff_microsecond = 1000;
	}
	struct timespec ts;
	ts.tv_sec = static_cast<time_t>(diff_microsecond/TimeStamp::kMicrosecondPerSecond);
	ts.tv_nsec =
	    static_cast<long>((diff_microsecond%TimeStamp::kMicrosecondPerSecond) * 1000);
	struct itimerspec new_value;
	bzero(&new_value, sizeof new_value);
	new_value.it_value = ts;
	if(::timerfd_settime(timer_fd_, 0, &new_value, nullptr) == -1)
	{
		LOG_ERROR("timerfd_settime error");
	}
}

TimerQueue::~TimerQueue()
{
	// Always set requested event to none before RemoveChannel().
	timer_fd_channel_.set_requested_event(Channel::NONE_EVENT);
	timer_fd_channel_.RemoveChannel();
	::close(timer_fd_);
	// TODO: If not use shared_ptr(cost too much) or unique_ptr(const property of
	// set element), use what to avoid `delete Timer*;` by ourself?
	for(ExpirationTimerPairSet::iterator it = active_timer_set_.begin();
	        it != active_timer_set_.end();
	        ++it)
	{
		delete it->second;
		// it->second = nullptr; error: assignment of member
		// ‘pair<TimeStamp, Timer*>::second’ in read-only object
	}
}

TimerId TimerQueue::AddTimer(const TimerCallback &callback,
                             const TimeStamp &expired_time,
                             double interval)
{
	Timer *timer = new Timer(callback, expired_time, interval);
	owner_loop_->RunInLoop(bind(&TimerQueue::AddTimerInLoop, this, timer));
	return TimerId(timer, timer->sequence());
}
void TimerQueue::AddTimerInLoop(Timer *timer)
{
	owner_loop_->AssertInLoopThread();
	if(InsertIntoActiveTimerSet(timer) == true)
	{
		SetExpiredTime(timer->expired_time());
	}
}

void TimerQueue::CancelTimer(const TimerId &timer_id)
{
	owner_loop_->RunInLoop(bind(&TimerQueue::CancelTimerInLoop, this, timer_id));
}
void TimerQueue::CancelTimerInLoop(const TimerId &timer_id)
{
	owner_loop_->AssertInLoopThread();

	Timer *timer = timer_id.timer_;
	ExpirationTimerPairSet::iterator it =
	    active_timer_set_.find(ExpirationTimerPair(timer->expired_time(), timer));
	if(it != active_timer_set_.end())
	{
		active_timer_set_.erase(it);
		delete timer;
	}
	else
	{
		canceling_timer_set_.insert(timer->sequence());
	}
}
