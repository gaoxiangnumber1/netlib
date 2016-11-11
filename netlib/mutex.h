#ifndef NETLIB_NETLIB_MUTEX_H_
#define NETLIB_NETLIB_MUTEX_H_

#include <assert.h> // assert()
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
class MutexLock: public NonCopyable
{
	friend class Condition; // TODO: Why make Condition as friend?
	// Me: Since we use MuetxLock private member(UnassignHolderGuard) in
	// Condition::Wait(), friend class can access the private member.
	// Class can allow another class or function to access its nonpublic members
	// by making that class or function a friend.
	// private member: base class itself and friend can access.
	// protected members: base class itself, friend and derived classes can access.

public:
	MutexLock(): holder_(0) // pthread_mutex_init
	{
		// int pthread_mutex_init(pthread_mutex_t *mutex,
		//                        const pthread_mutexattr_t *attr);
		// int pthread_mutex_destroy(pthread_mutex_t *mutex);
		// Both return: 0 if OK, error number on failure
		assert(pthread_mutex_init(&mutex_, NULL) == 0);
	}
	~MutexLock() // pthread_mutex_destroy
	{
		assert(holder_ == 0);
		assert(pthread_mutex_destroy(&mutex_) == 0);
	}

	// Used in `pthread_cond_wait(pthread_cond_t*, pthread_mutex_t*);` to get
	// condition_'s mutex_ object's pthread_mutex_t attribute.
	// Note this getter is non-const since we will change the value of mutex_
	// in pthread_cond_wait().
	pthread_mutex_t *get_pthread_mutex_t()
	{
		return &mutex_;
	}

	// Must be called when locked, i.e. for assertion.
	bool IsLockedByThisThread() const
	{
		return holder_ == Thread::ThreadId();
	}
	void AssertLockedByThisThread() const // TODO: Used in ThreadPool class.
	{
		assert(IsLockedByThisThread());
	}
	// Internal usage.
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

private:
	void AssignHolder()
	{
		holder_ = Thread::ThreadId();
	}
	void UnassignHolder()
	{
		holder_ = 0;
	}

	class UnassignHolderGuard: public NonCopyable // TODO: Used in Condition class.
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
		// Note: Since we use this class to guard the calling class's MutexLock
		// object is assign/un-assign, we should use `MutexLock&` as data member,
		// not `MutexLock` because the former will change calling class's MutexLock,
		// which is we expected, the latter will copy and change the copied, not original
		// MutexLock, thus it is wrong.
		MutexLock &owner_;
	};

	pthread_mutex_t mutex_;
	pid_t holder_; // TODO: Used in implementing ThreadPool class.
};

// Use as a stack variable, for example:
//		int Test::size() const
//		{
//			MutexLockGuard lock(mutex_);
//			return data_.size();
//		}
class MutexLockGuard: public NonCopyable
{
public:
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

// Prevent misuse like: `MutexLockGuard(mutex_);`
// A temporary object doesn't hold the lock for long! We should use stack object.
#define MutexLockGuard(name) error "Missing guard object name"

#endif  // NETLIB_NETLIB_MUTEX_H_
