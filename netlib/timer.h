#ifndef NETLIB_NETLIB_TIMER_H_
#define NETLIB_NETLIB_TIMER_H_

#include <netlib/callback.h>
#include <netlib/non_copyable.h>
#include <netlib/time_stamp.h>

namespace netlib
{

// Interface:
// Ctor
// Getter: expired_time, repeat, sequence
// Restart
// Run

class Timer: public NonCopyable
{
public:
	Timer(const TimerCallback &callback, TimeStamp time_stamp, double interval);
	// TODO: Timer(TimerCallback &&).
	// Getter
	TimeStamp expired_time() const
	{
		return expired_time_;
	}
	bool repeat() const // true if interval_ > 0.0
	{
		return repeat_;
	}
	int64_t sequence() const
	{
		return sequence_;
	}

	void Restart(TimeStamp now); // Restart timer from now on if interval_ > 0.0.
	void Run() const; // Run the event callback.

private:
	TimeStamp expired_time_; // Absolute expiration time.
	int64_t sequence_;
	TimerCallback callback_;
	double interval_;
	bool repeat_;
	static int64_t created_timer_number_; // FIXME: Atomic
};

}

#endif // NETLIB_NETLIB_TIMER_H_
