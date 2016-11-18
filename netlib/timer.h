#ifndef NETLIB_NETLIB_TIMER_H_
#define NETLIB_NETLIB_TIMER_H_

#include <netlib/callback.h>
#include <netlib/non_copyable.h>
#include <netlib/time_stamp.h>

#include <atomic> // atomic<>

namespace netlib
{

// Internal class for timer event.
class Timer: public NonCopyable
{
public:
	Timer(const TimerCallback &callback, TimeStamp time_stamp, double interval):
		callback_(callback),
		expired_time_(time_stamp),
		interval_(interval),
		repeat_(interval_ > 0.0),
		sequence_(++created_timer_number_) // Increment and get.
	{}
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
	void Run() const // Run the event callback.
	{
		callback_();
	}

private:
	const TimerCallback callback_; // Called in TimerQueue::HandleRead().
	TimeStamp expired_time_; // The absolute expiration time.
	// The time difference between each two expiration time for RunEvery().
	const double interval_;
	const bool repeat_; // true if interval_ > 0.0; false otherwise.
	const int64_t sequence_; // The global unique number to identify this timer.
	static std::atomic<int64_t> created_timer_number_;
};

}

#endif // NETLIB_NETLIB_TIMER_H_
