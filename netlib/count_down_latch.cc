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
		condition_.NotifyAll();
	}
}

void CountDownLatch::Wait()
{
	MutexLockGuard lock(mutex_);
	// NOTE: `while(count_ != 0)` is wrong since we only wait for count_ times.
	while(count_ > 0)
	{
		condition_.Wait();
	}
}
