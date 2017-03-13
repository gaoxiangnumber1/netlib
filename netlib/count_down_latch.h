#ifndef NETLIB_NETLIB_COUNT_DOWN_LATCH_H_
#define NETLIB_NETLIB_COUNT_DOWN_LATCH_H_

#include <netlib/condition.h>
#include <netlib/mutex.h>
#include <netlib/non_copyable.h>

namespace netlib
{

// Review: none.

// Interface:
// Ctor
// CountDown
// Wait

class CountDownLatch: public NonCopyable
{
public:
	explicit CountDownLatch(int count);

	void CountDown();
	void Wait();

private:
	// mutex_ Must before condition_ since we use mutex_ to construct condition_.
	MutexLock mutex_;
	Condition condition_;
	int count_; // The number of count down before Wait() return.
};

}

#endif // NETLIB_NETLIB_COUNT_DOWN_LATCH_H_
