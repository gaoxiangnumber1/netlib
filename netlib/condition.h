#ifndef NETLIB_NETLIB_CONDITION_H_
#define NETLIB_NETLIB_CONDITION_H_

#include <netlib/mutex.h> // MutexLock class.

namespace netlib
{

// Review: Wait, Signal, Broadcast

// Interface:
// Ctor
// Dtor
// Wait
// Signal
// Broadcast
class Condition: public NonCopyable
{
public:
	explicit Condition(MutexLock &mutex): mutex_(mutex)
	{
		assert(pthread_cond_init(&condition_, NULL) == 0);
	}
	~Condition()
	{
		assert(pthread_cond_destroy(&condition_) == 0);
	}

	// 1.	Thread X gets the lock of mutex_.
	// 2.	Thread X calls Condition::Wait(). In Wait(): pthread_cond_wait() atomically
	//		put Thread X on the waiting queue of condition and unlock the mutex.
	// 3.	Thread Y gets the lock of mutex_ -> changes the state of mutex_ -> calls
	//		Condition::Signal() or Condition::Broadcast() -> unlock the mutex_.
	// 4.	Thread X returns from `pthread_cond_wait()` and gets the lock of mutex_ again.
	void Wait()
	{
		mutex_.AssertLockedByThisThread();
		MutexLock::UnassignHolderGuard guard(mutex_); // mutex_.holder_ = 0;
		assert(pthread_cond_wait(&condition_, mutex_.get_pthread_mutex_t()) == 0);
	}
	void Signal()
	{
		assert(pthread_cond_signal(&condition_) == 0);
	}
	void Broadcast()
	{
		assert(pthread_cond_broadcast(&condition_) == 0);
	}

private:
	MutexLock &mutex_; // Condition use MutexLock not own it.
	pthread_cond_t condition_; // Protected by mutex_.
};

}

#endif // NETLIB_NETLIB_CONDITION_H_
