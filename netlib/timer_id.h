#ifndef NETLIB_NETLIB_TIMER_ID_H_
#define NETLIB_NETLIB_TIMER_ID_H_

#include <netlib/copyable.h>

namespace netlib
{

class Timer;

// Interface:
// Ctor

class TimerId: public Copyable
{
	friend class TimerQueue;
public:
	explicit TimerId(Timer *timer, int64_t sequence):
		timer_(timer),
		sequence_(sequence)
	{}

private:
	// Distinguish different Timer object by two values: <Timer*:sequence_number>.
	// Only use Timer* is not enough since we can't distinguish two different objects that
	// have the same memory address, which can happen if they are created at different
	// time(malloc -> free -> malloc). So we add a sequence number that increases
	// by 1 every time we create a new Timer object.
	Timer *timer_;
	// TODO: If two different Timer object have same address, then one must be deleted,
	// the other is alive. How to differentiate them if not use sequence_?
	int64_t sequence_;
};

}

#endif // NETLIB_NETLIB_TIMER_ID_H_
