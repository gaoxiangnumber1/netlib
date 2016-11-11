#ifndef NETLIB_NETLIB_CONDITION_H_
#define NETLIB_NETLIB_CONDITION_H_

// assert() and pthread_* already are in mutex.h.
#include <netlib/mutex.h> // MutexLock class.

namespace netlib
{

// When used with mutexes, condition variables allow threads to wait
// in a race-free way for arbitrary conditions to occur.
// The condition itself is protected by a mutex. A thread must first lock the mutex to
// change the condition state. Other threads will not notice the change until they acquire
// the mutex, because the mutex must be locked to be able to evaluate the condition.
class Condition
{
public:
	explicit Condition(MutexLock &mutex): mutex_(mutex) // pthread_cond_init
	{
		// int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
		// int pthread_cond_destroy(pthread_cond_t *cond);
		// Both return: 0 if OK, error number on failure.
		assert(pthread_cond_init(&condition_, NULL) == 0);
	}
	~Condition() // pthread_cond_destroy
	{
		assert(pthread_cond_destroy(&condition_) == 0);
	}

	void Wait() // TODO: how to use?
	{
		// TODO: why mutex_holder = 0;
		MutexLock::UnassignHolderGuard guard(mutex_); // mutex_.holder_ = 0;
		// int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
		// The argument `mutex` protects the condition. The caller passes it locked to the
		// function, which then atomically places the calling thread on the list of threads
		// waiting for the condition and unlocks the mutex. This closes the window between
		// the time that the condition is checked and the time that the thread goes to sleep
		// waiting for the condition to change, so that the thread doesn’t miss a change in
		// the condition. When this function returns, the mutex is again locked.
		// Return 0 if OK, error number on failure.
		assert(pthread_cond_wait(&condition_, mutex_.get_pthread_mutex_t()) == 0);
	}
	void Notify() // TODO: how to use?
	{
		// int pthread_cond_signal(pthread_cond_t *cond);
		// int pthread_cond_broadcast(pthread_cond_t *cond);
		// Both return: 0 if OK, error number on failure
		// pthread_cond_signal will wake up at least one thread waiting on a condition.
		// pthread_cond_broadcast will wake up all threads waiting on a condition.
		assert(pthread_cond_signal(&condition_) == 0);
	}
	void NotifyAll() // TODO: how to use?
	{
		assert(pthread_cond_broadcast(&condition_) == 0);
	}

private:
	MutexLock &mutex_; // Must `MutexLock&` not `MuetxLock`!
	pthread_cond_t condition_; // Protected by mutex_.
};

}

#endif // NETLIB_NETLIB_CONDITION_H_
