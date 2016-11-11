#ifndef NETLIB_NETLIB_THREAD_H_
#define NETLIB_NETLIB_THREAD_H_

#include <pthread.h> // pthread_t
#include <stdint.h> // int32_t
#include <sys/types.h> // pid_t

#include <atomic> // atomic<>
#include <functional> // function<>

#include <netlib/non_copyable.h> // NonCopyable

namespace netlib
{

class Thread: public NonCopyable
{
public:
	using ThreadFunction = std::function<void()>;

	explicit Thread(const ThreadFunction&);
	explicit Thread(ThreadFunction&&);
	~Thread(); // May call pthread_detach.

	static int created_number()
	{
		return created_number_.load();
	}
	int Join(); // Call pthread_join.
	void Start(); // Call pthread_create.
	static pid_t ThreadId(); // Get current thread's kernel-thread-id.

private:
	// In Ctor(): initialize to false; In Start(): assign `true` if pthread_create() success;
	// `false` otherwise. Other uses are for assertion.
	bool started_;
	// In Ctor(): initialize to `false`; In Join(): assign `true`. Other uses are for assertion.
	bool joined_;
	// POSIX thread ID, the opaque value returned by pthread_self(3).
	// Different threads in the different processes may have the same pthread_id.
	// Used in pthread_*().
	pthread_t pthread_id_;
	// For single-threaded process, the thread ID is equal to the process ID(getpid(2)).
	// For multi-threaded process, all threads have the same PID, but each one has
	// a unique TID. Thread ID's data type is pid_t. Thread IDs are unique system-wide,
	// and the kernel guarantees that no thread ID will be the same as any process ID
	// on the system, except when a thread is the thread group leader for a process.
	// glibc doesn't provide gettid(), using syscall(SYS_gettid).
	// TODO: why muduo use shared_ptr to manage thread id? I think it is not
	// necessary, so I simply use pid_t. To be honest, I haven't known the use of
	// this data member though it is an important attribute of thread.
	pid_t thread_id_;
	ThreadFunction function_; // Start function.
	static std::atomic<int32_t> created_number_; // The number of created threads.
};
}

#endif // NETLIB_NETLIB_THREAD_H_
