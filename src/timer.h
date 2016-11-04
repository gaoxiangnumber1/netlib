#ifndef NETLIB_SRC_TIMER_H_
#define NETLIB_SRC_TIMER_H_

#include <callback.h>
#include <non_copyable.h>
#include <time_stamp.h>

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
		repeat_(interval_ > 0.0)
	{}
	// Getter
	TimerCallback callback() const
	{
		return callback_;
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
	const TimerCallback callback_; // Called in TimerQueue::ReadCallback().
	TimeStamp expiration_; // The absolute expiration time for this timer.
	// The time "length" of expiration time, should equal to `expiration_ - Now()`
	const double interval_;
	const bool repeat_; // true if interval_ > 0.0; false otherwise.
};

}

#endif // NETLIB_SRC_TIMER_H_
