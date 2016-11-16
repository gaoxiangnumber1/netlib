#include <netlib/count_down_latch.h>

using netlib::CountDownLatch;
using netlib::MutexLockGuard;

CountDownLatch::CountDownLatch(int count):
	mutex_(),
	condition_(mutex_),
	count_(count)
{}

void CountDownLatch::CountDown()
{
	MutexLockGuard lock(mutex_);
	--count_;
	if(count_ == 0)
	{
		// Broadcast indicates state change rather than resource availability.
		condition_.NotifyAll();
	}
}

void CountDownLatch::Wait()
{
	// Must first get lock and then Wait() on condition.
	MutexLockGuard lock(mutex_);
	// NOTE: `while(count_ != 0)` is wrong since we only wait for initial count_ times.
	while(count_ > 0)
	{
		condition_.Wait();
	}
}
