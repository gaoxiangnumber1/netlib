#ifndef NETLIB_NETLIB_MUTEX_H_
#define NETLIB_NETLIB_MUTEX_H_

#include <assert.h> //  assert()
#include <pthread.h> // pthread_*
#include <sys/types.h> // pid_t

#include <netlib/non_copyable.h> // NonCopyable class.
#include <netlib/thread.h> // Thread::ThreadId();

namespace netlib
{

// Use as data member of a class, for example:
//		class Test
//		{
//		public:
//			int size() const;
//
//		private:
//			mutable MutexLock mutex_;
//			std::vector<int> data_; // Guard by mutex_
//		};

// Interface:
// Ctor.
// Dtor.
// IsLockedByThisThread.
// AssertLockedByThisThread -> +IsLockedByThisThread.

// For friend class MutexLockGuard:
// Lock -> -AssignHolder
// Unlock -> -UnassignHolder

// For friend class Condition:
// get_pthread_mutex_t
// class: UnassignHolderGuard
class MutexLock: public NonCopyable
{
	// NOTE: friend class declaration!
	friend class Condition; // Use private member class UnassignHolder.
	friend class MutexLockGuard; // Use private: Lock(), Unlock().
	// Private member: base class itself and friend can access.
	// Protected member: base class itself, friend and derived classes can access.

public:
	MutexLock(): holder_(0)
	{
		// int pthread_mutexattr_init(pthread_mutexattr_t *attr);
		// int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);
		// Both return: 0 if OK, error number on failure
		// pthread_mutexattr_init will initialize the pthread_mutexattr_t structure
		// with the default mutex attributes.
		// It is not necessary to use non-debug assert since I won't release build.
		assert(pthread_mutexattr_init(&mutex_attribute_) == 0);
		// int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);
		// Return: 0 if OK, error number on failure
		assert(pthread_mutexattr_settype(&mutex_attribute_,
		                                 PTHREAD_MUTEX_NORMAL) == 0);
		// int pthread_mutex_init(pthread_mutex_t *mutex,
		//                        const pthread_mutexattr_t *attr);
		// int pthread_mutex_destroy(pthread_mutex_t *mutex);
		// Both return: 0 if OK, error number on failure
		assert(pthread_mutex_init(&mutex_, &mutex_attribute_) == 0);
	}
	~MutexLock()
	{
		assert(holder_ == 0);
		assert(pthread_mutexattr_destroy(&mutex_attribute_) == 0);
		assert(pthread_mutex_destroy(&mutex_) == 0);
	}

	// Must be called when locked, i.e. for assertion.
	bool IsLockedByThisThread() const
	{
		return holder_ == Thread::ThreadId();
	}
	// Used in ThreadPool::IsTaskQueueFull(). Though it is not essential to check,
	// it is better if it checks invariant.
	void AssertLockedByThisThread() const
	{
		assert(IsLockedByThisThread());
	}

private:
	void AssignHolder() // Called when get the lock.
	{
		holder_ = Thread::ThreadId();
	}
	void UnassignHolder() // Called when release the lock.
	{
		holder_ = 0;
	}

	// Lock() and Unlock() can be used only by MutexLockGuard class.
	void Lock()
	{
		// int pthread_mutex_lock(pthread_mutex_t *mutex);
		// int pthread_mutex_unlock(pthread_mutex_t *mutex);
		// All return: 0 if OK, error number on failure
		assert(pthread_mutex_lock(&mutex_) == 0);
		AssignHolder();
	}
	void Unlock()
	{
		UnassignHolder();
		assert(pthread_mutex_unlock(&mutex_) == 0);
	}

	// get_pthread_mutex_t() and UnassignHolderGuard class
	// can be used only by Condition class.
	// Used in `pthread_cond_wait(pthread_cond_t*, pthread_mutex_t*);` to get the
	// condition_'s mutex_ object's pthread_mutex_t attribute.
	// This getter is non-const since pthread_cond_wait() changes the value of mutex_.
	pthread_mutex_t *get_pthread_mutex_t() // NOTE: Used by Condition::Wait()
	{
		return &mutex_;
	}
	// Called in Condition::Wait() just before calling `pthread_cond_wait()` because
	// this system call atomically puts the calling thread in the condition's waiting
	// queue and unlock the mutex, thus the calling thread doesn't own the lock of mutex,
	// thus we should un-assign the holder of mutex_ before `pthread_cond_wait()`.
	class UnassignHolderGuard: public NonCopyable
	{
	public:
		UnassignHolderGuard(MutexLock &owner): owner_(owner)
		{
			owner_.UnassignHolder();
		}
		~UnassignHolderGuard()
		{
			owner_.AssignHolder();
		}

	private:
		// NOTE: must use `MutexLock&` not `MutexLock` because the former changes
		// the calling thread's MutexLock, which is expected, the latter will copy and
		// change the copied, not original MutexLock, thus is wrong.
		MutexLock &owner_;
	};

	pthread_mutex_t mutex_;
	pthread_mutexattr_t mutex_attribute_;
	// The thread id of thread that holds this mutex_ lock.
	// 1.	Initialize to 0 in constructor.
	// 2.	Assign value by AssignHolder() each time we get the mutex_ lock.
	//		(1).	Lock(): by `MutexLockGuard lock(mutex_)`.
	//		(2).	~UnassignHolderGuard(): when Condition::Wait() returns.
	// 3.	Un-assign value by UnassignHolder() each time we release the mutex_ lock.
	//		(1).	Unlock(): by `~MutexLockGuard()`.
	//		(2).	UnassignHolderGuard(): when in Condition::Wait() just before calling
	//				pthread_cond_wait(), because this system call will atomically put calling
	//				thread into the condition wait queue and unlock mutex_, so the calling
	//				thread doesn't own this mutex_ lock, thus UnassignHolder().
	// 4.	Use for assertion: ~MutexLock() and IsLockedByThisThread().
	int holder_;
};

// Use as a stack variable, for example:
//		int Test::size() const
//		{
//			MutexLockGuard lock(mutex_);
//			return data_.size();
//		}

// Interface:
// Ctor
// Dtor
class MutexLockGuard: public NonCopyable
{
public:
	// NOTE: Must use `explicit` for MutexLockGuard constructor!
	// NOTE: The constructor must pass argument by Reference!
	explicit MutexLockGuard(MutexLock &mutex): mutex_(mutex)
	{
		mutex_.Lock();
	}
	~MutexLockGuard()
	{
		mutex_.Unlock();
	}

private:
	MutexLock &mutex_; // Must be `MutexLock&` not `MutexLock`.
};

}

// Prevent misusing like this: `MutexLockGuard(mutex_);` A temporary object
// is created and destroyed immediately after this expression, so we don't get the
// lock for the critical section. We should use stack object.
#define MutexLockGuard(name) error "Missing guard object name"

#endif  // NETLIB_NETLIB_MUTEX_H_
