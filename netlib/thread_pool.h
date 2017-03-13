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

// Review: Stop()

// Interface:
// Ctor
// Dtor -> +Stop
// Start -> -RunInThread -> -GetAndRemoveTask
// RunOrAddTask -> -IsTaskQueueFull

class ThreadPool: public NonCopyable
{
public:
	using Task = std::function<void()>;

	// NOTE: Use CONST whenever possible!
	explicit ThreadPool(const int thread_number,
	                    const Task &initial_task,
	                    const int max_queue_size);
	~ThreadPool();
	// Stop all threads and call Join() for all threads(all threads can't run again).
	void Stop();

	// Create thread_number_ threads and start all threads.
	void Start();
	// Run task() if thread_number_ is 0; otherwise add task into task queue.
	void RunOrAddTask(const Task &task);
	// TODO: C++11 `void RunOrAddTask(Task &&task);`

private:
	// The start function of thread. Start() -> RunInThread().
	void RunInThread();
	Task GetAndRemoveTask();
	// Check whether task_queue_ is full, that is, task_queue_.size() >= max_queue_size_.
	// max_queue_size_ is set by user. Called in RunOrAddTask() when adding task.
	bool IsTaskQueueFull() const;

	const int thread_number_;
	std::vector<Thread*> thread_pool_; // Store thread_number_ threads' pointer.
	const Task initial_task_; // The first task(i.e., function) that thread will run.
	bool running_; // Indicate the status of all threads.
	MutexLock mutex_; // Protect Condition and task queue.
	std::deque<Task> task_queue_;
	const int max_queue_size_;
	// 1.	GetAndRemoveTask(): `not_empty_.Wait()` -> `not_full_.Notify()`
	// 2.	RunOrAddTask(): `not_full_.Wait()` -> `not_empty_.Notify()`
	// 3.	~Dtor() -> Stop(): `not_empty_.NotifyAll()`.
	Condition not_empty_; // Wait when task_queue_ is empty.
	Condition not_full_; // Wait when task_queue_ if full.
};

}

#endif // NETLIB_NETLIB_THREAD_POOL_H_
