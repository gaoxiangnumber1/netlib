#ifndef NETLIB_NETLIB_TIMER_QUEUE_H_
#define NETLIB_NETLIB_TIMER_QUEUE_H_

#include <memory>
#include <set>
#include <vector>

#include <netlib/callback.h>
#include <netlib/channel.h>
#include <netlib/non_copyable.h>
#include <netlib/time_stamp.h>

namespace netlib
{

class EventLoop;
class Timer;
class TimerId;

// A best effort尽最大努力 timer queue. No guarantee that the callback will be on time.
class TimerQueue: public NonCopyable
{
public:
	TimerQueue(EventLoop *owner_loop);
	~TimerQueue();

	// Schedule the callback to be run at given time, repeats if `interval > 0.0`.
	// Thread safe: always add timer in the loop thread by calling
	// RunInLoop(AddTimerInLoop).
	// Used by EventLoop, and EventLoop encapsulates it to be RunAt(), RunAfter()...
	// Construct a new timer based on the arguments; Insert it to timer set;
	// Return a TimerId object that encapsulates this timer.
	TimerId AddTimer(const TimerCallback &callback,
	                 TimeStamp expired_time,
	                 double interval);
	void Cancel(TimerId timer_id);

private:
	// Don't use unique_ptr<Timer>:
	// 1.	Both iterator and const_iterator types of set give us read-only access to
	//		the elements in the set. Thus, for timers that can still restart, we can't update
	//		their time-stamp value in set, we must delete them in the set(only erase the
	//		pointer, not delete the object), and insert the updated timer into set again.
	// 2.	We can't erase/insert elements in the loop that traverses the set, because this will
	//		make iterators invalid. We must first get a copy of all timers that have expired,
	//		erase them in the set, update the copy Timer*, and insert the updated Timer*.
	// 3.	unique_ptr's copy constructor is deleted, we can't copy a unique_ptr.
	// 4.	We can't use `u.release()`: Relinquish control of the pointer u had held;
	//		return the pointer u has held and make u null.
	//			Try 1:	`Timer *timer = (it->second).release();`
	//			Error:	passing ‘const unique_ptr<Timer>’ as ‘this’ argument of
	//						‘unique_ptr<_Tp, _Dp>::pointer unique_ptr<_Tp, _Dp>::release()
	//						discards qualifiers [-fpermissive]
	//			This error means we should pass a non-const unique_ptr to release().
	//			Try 2:	`Timer*timer=(static_cast<unique_ptr<Timer>>(it->second)).release();`
	//			Error:	use of deleted function ‘unique_ptr(const unique_ptr<_Tp,_Dp>&)
	//						/usr/include/c++/4.8/bits/unique_ptr.h:273:7: error: declared here
	//							unique_ptr(const unique_ptr&) = delete;
	//			So, we can't get the non-const unique_ptr from const version unique_ptr in set.
	// 5.	The Dirty method is using get() to get the raw pointer of this unique_ptr<Timer>,
	//		create a new Timer object that is the updated object of this Timer. But this leads to
	//		too many timer objects and waste a lot of memory and lower performance.

	using TimerVector = std::vector<Timer*>;
	using ExpirationTimerPair = std::pair<TimeStamp, Timer*>;
	using ExpirationTimerPairSet = std::set<ExpirationTimerPair>;
	using TimerSequencePair = std::pair<Timer*, int64_t>;
	using TimerSequencePairSet = std::set<TimerSequencePair>;

	// Add timer in the loop thread. Always as a functor passed to RunInLoop().
	void AddTimerInLoop(Timer *timer);
	// Create a new timer fd. Called by TimerQueue::TimerQueue(EventLoop *loop).
	int CreateTimerFd();
	// Get the expired timers relative to `now` and store them in expired_time_ vector.
	void GetExpiredTimer(TimeStamp now);
	// Insert the specified timer into timer set. Return true if this timer will expire first.
	bool InsertIntoActiveTimerSet(Timer *timer);
	// The callback for IO read event, in this case, the timer fd alarms.
	void HandleRead();
	// Call ::read to read from `timer_fd` at `time_stamp` time.
	void ReadTimerFd(TimeStamp time_stamp);
	// Restart or delete expired timer and update timer_fd_'s expiration time.
	void Refresh(TimeStamp now);
	// Set timer_fd_'s expiration time to be `expiration` argument.
	void SetExpirationTime(TimeStamp expiration);
	void CancelInLoop(TimerId timer_id);

	EventLoop *owner_loop_; // Its owner loop.
	const int timer_fd_; // The timer file descriptor of this Timer object.
	Channel timer_fd_channel_; // Monitor the IO(readable) events on timer_fd_.
	// Store the expired Timer object's pointer for specified time.
	// We use it to call each Timer's callback and update the active_timer_set_by_expiration_.
	TimerVector expired_timer_;
	// Active Timer set sorted by <timer_expiration_time, timer_object_address>:
	// first sorted by the timer's expiration time; if two or more timers have the same
	// expiration time, we distinguish them by their object's address value.
	ExpirationTimerPairSet active_timer_set_by_expiration_;
	// For Cancel():
	bool calling_expired_timer_; // Atomic.
	// Active timer set sorted by <timer_object_address, sequence_number>:
	// first sorted by timer object's address, if two objects have same address value,
	// then sorted by sequence number.
	TimerSequencePairSet active_timer_set_by_address_;
	// Timer set that stores the canceled timer.
	TimerSequencePairSet canceling_timer_set_;
};

}

#endif // NETLIB_NETLIB_TIMER_QUEUE_H_
