#ifndef NETLIB_SRC_THREAD_H_
#define NETLIB_SRC_THREAD_H_

#include <stdint.h> // int64_t
#include <sys/types.h> // pid_t
#include <pthread.h> // pthread_t

#include <atomic> // atomic<>
#include <functional> // function<>
#include <memory> // shared_ptr<>

#include <non_copyable.h> // NonCopyable

namespace netlib
{

pid_t ThreadId();
void SleepUsec(int64_t usec);

class Thread: public NonCopyable
{
public:
	using ThreadFunction = std::function<void()>;

	explicit Thread(const ThreadFunction&);
	explicit Thread(ThreadFunction&&);
	~Thread();

	void Start();
	int Join(); // return pthread_join()

	pid_t thread_id() const
	{
		return *shared_thread_id_;
	}
	static int created_number()
	{
		return created_number_.load();
	}

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
	ThreadFunction function_;
	static std::atomic<int32_t> created_number_;
};
}

#endif // NETLIB_SRC_THREAD_H_
