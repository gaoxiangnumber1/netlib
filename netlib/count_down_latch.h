#ifndef NETLIB_NETLIB_COUNT_DOWN_LATCH_H_
#define NETLIB_NETLIB_COUNT_DOWN_LATCH_H_

#include <netlib/condition.h>
#include <netlib/mutex.h>
#include <netlib/non_copyable.h>

namespace netlib
{

class CountDownLatch: public NonCopyable
{
public:
	explicit CountDownLatch(int number);
	int count() const;

	void CountDown(); // Get lock -> --count_.
	void Wait(); // Wait until count_ reaches 0.

private:
	// Data member sequence: mutex_ Must come first and then condition_
	// since we use mutex_ to construct condition_ variable.
	// Since we need first get lock and then return the count_ value in `int count() const`,
	// the mutex_ should be `mutable` because getting the lock changes its value.
	mutable MutexLock mutex_;
	Condition condition_;
	int count_; // The number of count down before Wait() return.
};

}

#endif // NETLIB_NETLIB_COUNT_DOWN_LATCH_H_
