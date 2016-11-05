#ifndef NETLIB_NETLIB_MUTEX_H_
#define NETLIB_NETLIB_MUTEX_H_

#include <assert.h> // assert()
#include <pthread.h> // pthread_*
#include <sys/types.h> // pid_t

#include <netlib/thread.h>
#include <netlib/non_copyable.h>

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
public:
	MutexLock(): holder_(0)
	{
		// int pthread_mutex_init(pthread_mutex_t *mutex,
		//                        const pthread_mutexattr_t *attr);
		// int pthread_mutex_destroy(pthread_mutex_t *mutex);
		// Both return: 0 if OK, error number on failure
		assert(pthread_mutex_init(&mutex_, NULL) == 0);
	}
	~MutexLock()
	{
		assert(holder_ == 0);
		assert(pthread_mutex_destroy(&mutex_) == 0);
	}

	void AssertLocked() const
	{
		assert(holder_ == netlib::Thread::ThreadId());
	}
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
	pthread_mutex_t *get_pthread_mutex_t()
	{
		return &mutex_;
	}

private:
	friend class Condition;

	class UnassignGuard: public NonCopyable
	{
	public:
		UnassignGuard(MutexLock &owner): owner_(owner)
		{
			owner_.UnassignHolder();
		}
		~UnassignGuard()
		{
			owner_.AssignHolder();
		}

	private:
		MutexLock &owner_;
	};

	void AssignHolder()
	{
		holder_ = netlib::Thread::ThreadId();
	}
	void UnassignHolder()
	{
		holder_ = 0;
	}

	pthread_mutex_t mutex_;
	pid_t holder_;
};

// Use as a stack variable, for example:
//		int Test::size() const
//		{
//			MutexLockGuard lock(mutex_);
//			return data_.size();
//		}

class MutexLockGuard : public NonCopyable
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
	MutexLock &mutex_;
};

}

// Prevent misuse like: `MutexLockGuard(mutex_);`
// A temporary object doesn't hold the lock for long!
#define MutexLockGuard(name) error "Missing guard object name"

#endif  // NETLIB_NETLIB_MUTEX_H_
