#ifndef NETLIB_NETLIB_TIMER_H_
#define NETLIB_NETLIB_TIMER_H_

#include <netlib/function.h>
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
	Timer(const TimerCallback &callback, const TimeStamp &time_stamp, double interval);

	// Getter
	TimeStamp expired_time() const
	{
		return expired_time_;
	}
	bool repeat() const
	{
		return repeat_;
	}
	int64_t sequence() const
	{
		return sequence_;
	}

	void Restart(const TimeStamp &now);
	void Run() const;

private:
	TimerCallback callback_;
	TimeStamp expired_time_; // Absolute expiration time.
	double interval_;
	bool repeat_;
	static int64_t created_timer_number_; // FIXME: Atomic
	int64_t sequence_;
};

}

#endif // NETLIB_NETLIB_TIMER_H_
