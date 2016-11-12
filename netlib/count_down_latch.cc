#include <netlib/count_down_latch.h>

using netlib::CountDownLatch;
using netlib::MutexLockGuard;

CountDownLatch::CountDownLatch(int number):
	mutex_(),
	condition_(mutex_),
	count_(number)
{}

void CountDownLatch::Wait()
{
	MutexLockGuard lock(mutex_);
	while(count_ > 0)
	{
		condition_.Wait();
	}
}

void CountDownLatch::CountDown()
{
	MutexLockGuard lock(mutex_);
	--count_;
	if(count_ == 0)
	{
		// broadcast should be used to indicate state change
		// rather than resource availability.
		condition_.NotifyAll();
	}
}

int CountDownLatch::count() const
{
	MutexLockGuard lock(mutex_);
	return count_;
}
