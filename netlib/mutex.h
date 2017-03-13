#ifndef NETLIB_NETLIB_MUTEX_H_
#define NETLIB_NETLIB_MUTEX_H_

#include <assert.h> //  assert()
#include <pthread.h> // pthread_*
#include <sys/types.h> // pid_t

#include <netlib/non_copyable.h> // NonCopyable class.
#include <netlib/thread.h> // Thread::ThreadId();

namespace netlib
{

// Use MutexLock as data member of a class, for example:
//		class Test
//		{
//		public:
//			int size() const;
//
//		private:
//			mutable MutexLock mutex_;
//			std::vector<int> data_; // Guard by mutex_
//		};
// Use MutexLockGuard as a stack variable, for example:
//		int Test::size() const
//		{
//			MutexLockGuard lock(mutex_);
//			return data_.size();
//		}

// Review: 0

// Interface:
// Ctor.
// Dtor.
// IsLockedByThisThread.
// AssertLockedByThisThread -> +IsLockedByThisThread.
// For friend MutexLockGuard: Lock -> -AssignHolder, Unlock -> -UnassignHolder
// For friend Condition: get_pthread_mutex_t, UnassignHolderGuard
class MutexLock: public NonCopyable
{
	friend class MutexLockGuard; // Use private: Lock(), Unlock().
	friend class Condition; // Use private member class UnassignHolder.
	// Private member: base class itself and friend can access.
	// Protected member: base class itself, friend and derived classes can access.

public:
	MutexLock(): holder_(0)
	{
		// No need use non-debug assert since I won't release build.
		assert(pthread_mutexattr_init(&mutex_attribute_) == 0);
		assert(pthread_mutexattr_settype(&mutex_attribute_,
		                                 PTHREAD_MUTEX_NORMAL) == 0);
		assert(pthread_mutex_init(&mutex_, &mutex_attribute_) == 0);
	}
	~MutexLock()
	{
		assert(holder_ == 0);
		assert(pthread_mutexattr_destroy(&mutex_attribute_) == 0);
		assert(pthread_mutex_destroy(&mutex_) == 0);
	}

	bool IsLockedByThisThread() const // NOTE: Use const whenever possible.
	{
		return holder_ == Thread::ThreadId();
	}
	// Used in ThreadPool::IsTaskQueueFull().
	void AssertLockedByThisThread() const
	{
		assert(IsLockedByThisThread() == true);
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

	void Lock()
	{
		assert(pthread_mutex_lock(&mutex_) == 0);
		AssignHolder();
	}
	void Unlock()
	{
		UnassignHolder();
		assert(pthread_mutex_unlock(&mutex_) == 0);
	}

	// Used in `pthread_cond_wait(pthread_cond_t*, pthread_mutex_t*);`
	// This getter is non-const since pthread_cond_wait() changes the value of mutex_.
	pthread_mutex_t *get_pthread_mutex_t()
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
		// NOTE: must use `MutexLock&` not `MutexLock`.
		// The latter copies and changes the copied, not original MutexLock, thus is wrong.
		MutexLock &owner_;
	};

	pthread_mutexattr_t mutex_attribute_;
	pthread_mutex_t mutex_;
	// The thread id of thread that holds this mutex_ lock.
	// 1.	Initialize to 0 in constructor.
	// 2.	Assign value by AssignHolder() each time we get the mutex_ lock.
	//		(1).	Lock(): by `MutexLockGuard lock(mutex_)`.
	//		(2).	~UnassignHolderGuard(): when Condition::Wait() returns.
	// 3.	Unassign value by UnassignHolder() each time we release the mutex_ lock.
	//		(1).	Unlock(): by `~MutexLockGuard()`.
	//		(2).	UnassignHolderGuard(): when in Condition::Wait() just before calling
	//				pthread_cond_wait(), because this system call atomically puts calling
	//				thread into the condition wait queue and unlock mutex_, so the calling
	//				thread doesn't own this mutex_ lock, thus UnassignHolder().
	// 4.	Use for assertion: ~MutexLock() and IsLockedByThisThread().
	int holder_;
};

// Interface:
// Ctor
// Dtor
class MutexLockGuard: public NonCopyable
{
public:
	// NOTE: Must use `explicit` for MutexLockGuard constructor!
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
// is created and destroyed immediately after this expression, so we don't acquire the
// lock for the critical section. We should use stack object.
#define MutexLockGuard(mutex_name) error "Missing guard object name"

#endif  // NETLIB_NETLIB_MUTEX_H_
