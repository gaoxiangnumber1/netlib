#ifndef NETLIB_NETLIB_TIMER_H_
#define NETLIB_NETLIB_TIMER_H_

#include <netlib/callback.h>
#include <netlib/non_copyable.h>
#include <netlib/time_stamp.h>

namespace netlib
{

// Internal class for timer event.
class Timer: public NonCopyable
{
public:
	Timer(const TimerCallback &fun, TimeStamp expired_time, double time):
		callback_(fun),
		expiration_(expired_time), // TODO: how to set expiration_ and interval_?
		interval_(time),
		repeat_(interval_ > 0.0),
		sequence_(++create_number_) // Increment and get.
	{}
	// Getter
	int64_t sequence() const
	{
		return sequence_;
	}
	TimeStamp expiration() const
	{
		return expiration_;
	}
	double interval() const
	{
		return interval_;
	}
	bool repeat() const // true if interval_ > 0.0
	{
		return repeat_;
	}

	void Run() const // Run the event callback.
	{
		callback_();
	}
	void Restart(TimeStamp now); // Restart timer from now on if interval_ > 0.0.

private:
	const TimerCallback callback_; // Called in TimerQueue::HandleRead().
	TimeStamp expiration_; // The absolute expiration time for this timer.
	// The time "length" of expiration time, should equal to `expiration_ - Now()`
	const double interval_;
	const bool repeat_; // true if interval_ > 0.0; false otherwise.
	const int64_t sequence_;

	static atomic<int64_t> create_number_;
};

}

#endif // NETLIB_NETLIB_TIMER_H_
