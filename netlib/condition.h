#ifndef NETLIB_NETLIB_CONDITION_H_
#define NETLIB_NETLIB_CONDITION_H_

#include <netlib/mutex.h>
#include <netlib/logging.h>

namespace netlib
{

class Condition
{
public:
	explicit Condition(MutexLock &mutex): mutex_(mutex)
	{
		// int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
		// int pthread_cond_destroy(pthread_cond_t *cond);
		// Both return: 0 if OK, error number on failure.
		assert(pthread_cond_init(&condition_, NULL) == 0);
	}
	~Condition()
	{
		assert(pthread_cond_destroy(&condition_) == 0);
	}

	void Wait()
	{
		// TODO: why mutex_holder = 0;
		MutexLock::UnassignGuard unassign_guard(mutex_); // mutex_.holder_ = 0;
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
	void Notify()
	{
		// int pthread_cond_signal (pthread_cond_t *cond);
		// int pthread_cond_broadcast(pthread_cond_t *cond);
		// Both return: 0 if OK, error number on failure
		// pthread_cond_signal will wake up at least one thread waiting on a condition.
		// pthread_cond_broadcast will wake up all threads waiting on a condition.
		assert(pthread_cond_signal(&condition_) == 0);
	}
	void NotifyAll()
	{
		assert(pthread_cond_broadcast(&condition_) == 0);
	}

private:
	MutexLock &mutex_;
	pthread_cond_t condition_;
};

}

#endif // NETLIB_NETLIB_CONDITION_H_
