#ifndef NETLIB_NETLIB_THREAD_POOL_H_
#define NETLIB_NETLIB_THREAD_POOL_H_

#include <deque>
#include <functional>
#include <vector>

#include <netlib/condition.h>
#include <netlib/mutex.h>
#include <netlib/non_copyable.h>

namespace netlib
{

// Review: Stop(), Start(), GetAndRemoveTask(), RunOrAddTask(), IsTaskQueueFull()

// Interface:
// Ctor
// Dtor -> +Stop
// Start -> -RunInThread -> -GetAndRemoveTask
// RunOrAddTask -> -IsTaskQueueFull

class ThreadPool: public NonCopyable
{
public:
	using ThreadTask = std::function<void()>;

	explicit ThreadPool(const int thread_number,
	                    const ThreadTask &initial_task,
	                    const int max_queue_size);
	~ThreadPool();
	// Stop all threads and call Join() for all threads(all threads can't run again).
	void Stop();

	// Create thread_number_ threads and start all threads.
	void Start();
	// Run task() if thread_number_ is 0; otherwise add task into task queue.
	void RunOrAddTask(const ThreadTask &task);
	// TODO: C++11 `void RunOrAddTask(ThreadTask &&task);`

private:
	// The start function of thread. Start() -> RunInThread().
	void RunInThread();
	ThreadTask GetAndRemoveTask();
	bool IsTaskQueueFull() const;

	const int thread_number_;
	std::vector<Thread*> thread_pool_; // Store thread_number_ threads' pointer.
	const ThreadTask initial_task_; // The first task that thread will run.
	bool running_; // Indicate the status of all threads.
	MutexLock mutex_; // Protect Condition and task queue.
	std::deque<ThreadTask> task_queue_;
	const int max_queue_size_;
	// 1.	GetAndRemoveTask(): `not_empty_.Wait()` -> `not_full_.Notify()`
	// 2.	RunOrAddTask(): `not_full_.Wait()` -> `not_empty_.Notify()`
	// 3.	~Dtor() -> Stop(): `not_empty_.NotifyAll()`.
	Condition not_empty_; // Wait when task_queue_ is empty.
	Condition not_full_; // Wait when task_queue_ if full.
};

}

#endif // NETLIB_NETLIB_THREAD_POOL_H_
