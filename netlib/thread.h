#ifndef NETLIB_NETLIB_THREAD_H_
#define NETLIB_NETLIB_THREAD_H_

#include <stdint.h> // int64_t
#include <sys/types.h> // pid_t
#include <unistd.h> // getpid(), may use.
#include <pthread.h> // pthread_t

#include <atomic> // atomic<>
#include <functional> // function<>
#include <memory> // shared_ptr<>

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

	bool started()
	{
		return started_;
	}

	void Start(); // Invoke pthread_create.
	int Join(); // Invoke pthread_join.

	static int created_number()
	{
		return created_number_.load();
	}
	static pid_t ThreadId(); // Get current thread's kernel-thread-id.

private:
	bool started_;
	bool joined_;
	// POSIX thread ID, the opaque value returned by pthread_self(3).
	// Different threads in the different processes may have the same pthread_id.
	pthread_t pthread_id_;
	// For single-threaded process, the thread ID is equal to the process ID(getpid(2)).
	// For multi-threaded process, all threads have the same PID, but each one has
	// a unique TID. Thread ID's data type is pid_t. Thread IDs are unique system-wide,
	// and the kernel guarantees that no thread ID will be the same as any process ID
	// on the system, except when a thread is the thread group leader for a process.
	// glibc doesn't provide gettid(), using syscall(SYS_gettid).
	std::shared_ptr<pid_t> shared_thread_id_;
	ThreadFunction function_; // Start function.
	static std::atomic<int32_t> created_number_; // The number of created threads.
};
}

#endif // NETLIB_NETLIB_THREAD_H_
