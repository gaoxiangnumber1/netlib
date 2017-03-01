#include <netlib/timer_queue.h>

#include <assert.h>
#include <string.h> // strerror()
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
	timer_fd_(CreateTimerFd()), // Create a new timer.
	timer_fd_channel_(owner_loop_, timer_fd_)
{
	// The function signature can be different. We can choose not use the original
	// signature's args, if so, the parameter(s) supplied were ignored.
	timer_fd_channel_.set_event_callback(Channel::READ_CALLBACK,
	                                     bind(&TimerQueue::HandleRead, this));
	timer_fd_channel_.set_requested_event(Channel::READ_EVENT);
}
// #include <sys/timerfd.h>
// int timerfd_create(int clockid, int flags);
// timerfd_create() creates a new timer object, and returns a file descriptor that refers to
// that timer. `clockid` specifies the clock that is used to mark the progress of the timer:
// 1. CLOCK_REALTIME: a settable system-wide clock.
// 2. CLOCK_MONOTONIC: a non-settable clock that is not affected by discontinuous
// changes in the system clock(e.g., manual changes to system time).(ZH:Dan Diao De)
// `flags`
// 1. TFD_NONBLOCK: Set `O_NONBLOCK` file status flag on the new file descriptor.
// 2. TFD_CLOEXEC: Set `close-on-exec(FD_CLOEXEC)` flag.
// Return a new file descriptor on success. -1 on error and errno is set.
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
// The callback for IO read event, in this case, the timer fd alarms.
void TimerQueue::HandleRead()
{
	owner_loop_->AssertInLoopThread();
	// 1. Get expired_time time and Invoke `ReadTimerFd()` to read form timer_fd_.
	TimeStamp expired_time(TimeStamp::Now());
	// Since we use level trigger, we should read timer_fd after it expired,
	// otherwise it would trigger immediately next time when using epoll_wait().
	ReadTimerFd(expired_time);
	// 2.	Get the expired timers relative to `expired_time` and
	//		store them in expired_time_vector_. Run each timer's callback_.
	GetAndRemoveExpiredTimer(expired_time);
	for(TimerVector::iterator it = expired_timer_vector_.begin();
	        it != expired_timer_vector_.end();
	        ++it)
	{
		(*it)->Run(); // Timer_Pointer->Run() runs timer object's callback_.
	}
	// 3. Refresh state for the next expiration.
	Refresh(expired_time);
}
// Call ::read to read from `timer_fd` at `time_stamp` time.
void TimerQueue::ReadTimerFd(TimeStamp time_stamp)
{
	// If the timer has expired one or more times since its settings were last modified using
	// timerfd_settime(), or since the last successful read(2), then the buffer given to read(2)
	// returns an unsigned 8-byte integer(uint64_t) containing the number of expirations
	// that have occurred. The returned value is in host byte order.
	uint64_t expired_number;
	int read_byte =
	    static_cast<int>(::read(timer_fd_, &expired_number, sizeof expired_number));
	LOG_TRACE("expired number = %d, time = %s",
	          static_cast<int>(expired_number),
	          time_stamp.ToFormattedTimeString().c_str());
	if(read_byte != sizeof expired_number)
	{
		LOG_ERROR("TimerQueue::ReadTimerFd read %d bytes instead of 8.", read_byte);
	}
}
// Get the expired timers relative to `expired_time` and
// store them in expired_time_vector_.
void TimerQueue::GetAndRemoveExpiredTimer(TimeStamp expired_time)
{
	// 1. Set sentry to search the set and get the first not expired timer iterator.
	// sentry is the biggest timer-pair whose time-stamp value is `expired_time`.
	// If the key is in the container, the iterator returned from lower_bound will refer to
	// the first instance of that key and the iterator returned by upper_bound will refer
	// just after the last instance of the key. If the element is not in the container, then
	// lower_bound and upper_bound will return equal iterators; both will refer to
	// the point at which the key can be inserted without disrupting the order.
	ExpirationTimerPair sentry(expired_time, reinterpret_cast<Timer*>(UINTPTR_MAX));
	ExpirationTimerPairSet::iterator first_not_expired =
	    active_timer_set_.lower_bound(sentry);
	//(1)	Not find the iterator that we can insert sentry into timer set, that is, all timers
	//		in timer set are already expired.
	//(2)	If found, the first not expired timer's expiration time must be greater than
	//		`expired_time` since sentry is the biggest timer whose expiration time is para.
	assert(first_not_expired == active_timer_set_.end() ||
	       expired_time < first_not_expired->first);

	// 2. Clear the old expired timer vector.
	expired_timer_vector_.clear();

	// 3. Copy all the expired timer's pointer from active_timer_set_
	// to expired_timer_vector_.
	for(ExpirationTimerPairSet::iterator it = active_timer_set_.begin();
	        it != first_not_expired;
	        ++it)
	{
		expired_timer_vector_.push_back(it->second); // Pass by value: copy a pointer.
	}

	// 4.	Erase the expired timers in the active_timer_set_, this don't
	//		destruct the timer. TODO: what happens when set.erase(iterator)?
	active_timer_set_.erase(active_timer_set_.begin(), first_not_expired);
}
// Restart or delete expired timer and update timer_fd_'s expiration time.
void TimerQueue::Refresh(TimeStamp expired_time)
{
	// 1. For expired timer:
	//		(1). Restart if it can repeat and not be canceled.
	//		(2). Delete it otherwise.
	for(TimerVector::iterator it = expired_timer_vector_.begin();
	        it != expired_timer_vector_.end();
	        ++it)
	{
		// NOTE: Only not_canceled timer can be inserted again!
		if((*it)->repeat() == true &&
		        canceling_timer_sequence_set_.find((*it)->sequence()) ==
		        canceling_timer_sequence_set_.end())
		{
			// Restart timer and insert the updated Timer* into timer pair set again.
			(*it)->Restart(expired_time);
			InsertIntoActiveTimerSet(*it);
		}
		else
		{
			delete (*it); // This timer can't repeat, delete it explicitly.
			(*it) = nullptr;
			// Since `expired_timer_vector_.clear()` at the begin of GARET(),
			// we don't need erase this deleted Timer* in expired_time_ vector.
		}
	}
	canceling_timer_sequence_set_.clear();
	// 2. NOTE: Set next expiration time if active timer set is not empty!
	if(active_timer_set_.empty() == false)
	{
		SetExpiredTime(active_timer_set_.begin()->first);
	}
}
// Insert the specified timer into timer set. Return true if this timer will expire first.
bool TimerQueue::InsertIntoActiveTimerSet(Timer *timer)
{
	owner_loop_->AssertInLoopThread();

	// Make a new pair for this timer and insert it to the timer set.
	TimeStamp expired_time = timer->expired_time();
	active_timer_set_.insert(ExpirationTimerPair(expired_time, timer));

	bool is_first_expired = false;
	if(expired_time == active_timer_set_.begin()->first)
	{
		is_first_expired = true;
	}
	return is_first_expired;
	// We don't call `SetExpiredTime()` in this function when `is_first_expired` is true.
	// Assume we insert 100 timers that each expired_time is smaller than the before
	// one, if we do SetExpiredTime() in this function, we will call timerfd_settime()
	// for 100 times, which costs too much. Instead, this function only return a flag
	// that indicates whether we need update expired time and leave the real update
	// decision to the caller. When we insert many timers, this can reduce the calling
	// of system call `timerfd_settime()` and improve performance.
}
// Set the expiration time to `expiration - Now()`. The argument `expiration` is the
// absolute expiration time, that is, there will have timers that expire at `expiration`
// seconds since Epoch.
void TimerQueue::SetExpiredTime(TimeStamp expiration)
{
	// 1. Convert `expiration` to a `struct timespec`
	int64_t microsecond = expiration.microsecond_since_epoch()
	                      - TimeStamp::Now().microsecond_since_epoch();
	// Suppose:
	//					RunEvery(callback, 1);	// Timer 1
	//					RunEvery(callback, 2);	// Timer 2
	//					RunAfter(callback, 2);	// Timer 3
	// At 1 second: timer 1 expires -> poll return -> callback() -> timer 1 restart to expire
	// at 2 second. Roughly speaking, timer 1, 2 and 3 will all expire at 2 second. But since
	// timer 1 runs callback and then set expiration time to 1 second later, its expiration
	// time is a little later than timer 2 and 3's expiration time, about tens of microseconds,
	// which is the following case. When we call `Refresh()->SetExpiredTime();`, the
	// timer 1 may have already expired(i.e., microsecond < 0) or about to expire
	// (i.e., 0 <= microsecond < 100). For both cases, we set the expiration time to 100
	// microsecond to let the timer expire almost instantly, thus poll will return and runs
	// this timer's callback instantly.
	if(microsecond < 100)
	{
		microsecond = 100;
	}
	struct timespec ts;
	ts.tv_sec = static_cast<time_t>(microsecond / TimeStamp::kMicrosecondPerSecond);
	ts.tv_nsec =
	    static_cast<long>((microsecond % TimeStamp::kMicrosecondPerSecond) * 1000);

	// 2. Call timerfd_settime to set the new expiration time for the timer set.
	struct itimerspec new_value;
	bzero(&new_value, sizeof new_value);
	new_value.it_value = ts;
	// int timerfd_settime(int fd, int flags, const struct itimerspec *new_value,
	//                     struct itimerspec *old_value);
	// flags = 0: start a relative timer. new_value.it_value specifies a time relative to
	// the current value of the clock specified by clockid.
	// Return 0 on success; -1 on error and set errno to indicate the error.
	if(::timerfd_settime(timer_fd_, 0, &new_value, NULL) == -1)
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
	// TODO: What's mean `Do not remove channel, since we're in EventLoop::dtor();`?
	// Me: This means don't `delete channel_`, otherwise double free?
	// TODO: If we don't use shared_ptr(since it cost too much) or unique_ptr
	// (since the const property of set element), we should use what to avoid
	// `delete Timer*;` by ourself?
	for(ExpirationTimerPairSet::iterator it = active_timer_set_.begin();
	        it != active_timer_set_.end();
	        ++it)
	{
		delete it->second;
		// it->second = nullptr; error: assignment of member ‘pair<TimeStamp, Timer*>
		// ::second’ in read-only object
	}
}

// Construct a new timer and Call RunInLoop to add timer to set in the loop thread.
// Return a TimerId object that encapsulates this timer.
TimerId TimerQueue::AddTimer(const TimerCallback &callback,
                             TimeStamp expired_time,
                             double interval)
{
	// 1. Create a Timer object based on arguments.
	Timer *timer = new Timer(callback, expired_time, interval);
	// 2. Add this new timer in loop thread by calling RunInLoop().
	owner_loop_->RunInLoop(bind(&TimerQueue::AddTimerInLoop, this, timer));
	// 3. Return the inserted timer as a TimerId object.
	return TimerId(timer, timer->sequence());
}
// Add timer in the loop thread. Always as a functor passed to RunInLoop().
void TimerQueue::AddTimerInLoop(Timer *timer)
{
	owner_loop_->AssertInLoopThread();
	// Insert this timer into timer set. Return true if this timer will expire first.
	// If this timer will expire first, update timer_fd_'s expiration time.
	if(InsertIntoActiveTimerSet(timer) == true)
	{
		SetExpiredTime(timer->expired_time());
	}
}

void TimerQueue::CancelTimer(TimerId timer_id)
{
	owner_loop_->RunInLoop(bind(&TimerQueue::CancelTimerInLoop, this, timer_id));
}
void TimerQueue::CancelTimerInLoop(TimerId timer_id)
{
	owner_loop_->AssertInLoopThread();
	Timer *timer = timer_id.timer_; // Friend class.
	ExpirationTimerPairSet::iterator it =
	    active_timer_set_.find(ExpirationTimerPair(timer->expired_time(), timer));
	if(it != active_timer_set_.end())
	{
		active_timer_set_.erase(it);
		delete timer; // NOTE: `delete Timer*;`!
		timer = nullptr;
	}
	else
	{
		canceling_timer_sequence_set_.insert(timer->sequence());
	}
}
