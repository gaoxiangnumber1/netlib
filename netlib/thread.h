#ifndef NETLIB_NETLIB_THREAD_H_
#define NETLIB_NETLIB_THREAD_H_

#include <pthread.h> // pthread_t

#include <functional> // function<>

#include <netlib/non_copyable.h> // NonCopyable

namespace netlib
{

// Review: Start, Join, Dtor

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
	using ThreadMainFunction = std::function<void()>;

	explicit Thread(const ThreadMainFunction&);
	// TODO: explicit Thread(ThreadMainFunction&&);
	~Thread(); // May call pthread_detach.

	static int created_number()
	{
		return created_number_;
	}

	void Start(); // Call pthread_create.
	static int ThreadId(); // Get current thread's kernel-thread-id.
	void Join(); // Call pthread_join.

private:
	bool started_;
	bool joined_;
	pthread_t pthread_id_; // POSIX thread ID returned by pthread_self(3).
	int thread_id_; // Can't be const.
	const ThreadMainFunction function_; // Start function.
	static int created_number_; // FIXME: Atomic
};
}

#endif // NETLIB_NETLIB_THREAD_H_
