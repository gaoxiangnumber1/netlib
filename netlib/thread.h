#ifndef NETLIB_NETLIB_THREAD_H_
#define NETLIB_NETLIB_THREAD_H_

#include <pthread.h> // pthread_t

#include <atomic> // atomic<>
#include <functional> // function<>

#include <netlib/non_copyable.h> // NonCopyable

namespace netlib
{

// Review:
// NonFunction: ThreadFunction, thread_id_#non_const, created_number_
// Function: StartThread#delete, ForkHandler#object, ChildForkHandler#void()

// Interface:
// Ctor
// Dtor
// created_number
// Start -> netlib::ThreadData -> netlib::StartThread
// ThreadId
// ForkHandler -> ChildForkHandler
// Join

class Thread: public NonCopyable
{
public:
	using ThreadFunction = std::function<void()>;

	explicit Thread(const ThreadFunction&);
	// TODO: C++11 explicit Thread(ThreadFunction&&);
	~Thread(); // May call pthread_detach.

	static int created_number()
	{
		return created_number_.load();
	}

	void Start(); // Call pthread_create.
	static int ThreadId(); // Get current thread's kernel-thread-id.
	void Join(); // Call pthread_join.

private:
	// 1.	Ctor(): initialize to false.
	// 2.	Start(): assign `true` if pthread_create() success; `false` otherwise.
	// 3.	Other uses are for assertion.
	bool started_;
	// 1.	Ctor(): initialize to `false`.
	// 2.	Join(): assign `true`.
	// 3.	Other uses are for assertion.
	bool joined_;
	// POSIX thread ID returned by pthread_self(3). Different threads in the different
	// processes may have the same pthread_id. Used in pthread_*().
	pthread_t pthread_id_;
	// For single-threaded process: thread ID = process ID(getpid(2)).
	// For multi-threaded process: all threads have the same PID, but each one has
	// a unique TID. Thread ID's data type is pid_t. Thread IDs are unique system-wide,
	// and the kernel guarantees that no thread ID is the same as any process ID
	// on the system, except when a thread is the thread group leader for a process.
	// glibc doesn't provide gettid(), using syscall(SYS_gettid).
	int thread_id_; // Can't be const.
	// TODO: why muduo use shared_ptr to manage thread id? I think it is not
	// necessary, so I simply use int(same as pid_t).
	// NOTE: Use const since we can't change function_ after creating the thread.
	const ThreadFunction function_; // Start function. Not `ThreadFunction&`
	static std::atomic<int> created_number_; // The number of created threads.
};
}

#endif // NETLIB_NETLIB_THREAD_H_
