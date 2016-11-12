#ifndef NETLIB_NETLIB_COUNT_DOWN_LATCH_H_
#define NETLIB_NETLIB_COUNT_DOWN_LATCH_H_

#include <netlib/non_copyable.h>
#include <netlib/mutex.h>
#include <netlib/condition.h>

namespace netlib
{

class CountDownLatch: public NonCopyable
{
public:
	explicit CountDownLatch(int number);
	void Wait(); // Wait until count_ reaches 0.
	void CountDown(); // Get lock -> --count_.
	int count() const;

private:
	// Must first mutex then condition since we use mutex to construct condition variable.
	mutable MutexLock mutex_;
	Condition condition_;
	int count_; // The number of count down before no need to Wait().
};

}

#endif // NETLIB_NETLIB_COUNT_DOWN_LATCH_H_
