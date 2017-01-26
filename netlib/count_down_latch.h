#ifndef NETLIB_NETLIB_COUNT_DOWN_LATCH_H_
#define NETLIB_NETLIB_COUNT_DOWN_LATCH_H_

#include <netlib/condition.h>
#include <netlib/mutex.h>
#include <netlib/non_copyable.h>

namespace netlib
{

// Interface:
// Ctor
// CountDown
// Wait

class CountDownLatch: public NonCopyable
{
public:
	explicit CountDownLatch(int count);

	void CountDown(); // Get lock -> --count_.
	void Wait(); // Wait until count_ reaches 0.

private:
	// Data member sequence: mutex_ Must come first and then condition_
	// since we use mutex_ to construct condition_ variable.
	// NOTE: CountDownLatch owns one MutexLock, so its data member isn't `ML&`.
	MutexLock mutex_;
	Condition condition_;
	int count_; // The number of count down before Wait() return.
};

}

#endif // NETLIB_NETLIB_COUNT_DOWN_LATCH_H_
