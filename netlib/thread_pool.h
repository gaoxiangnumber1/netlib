#ifndef NETLIB_NETLIB_THREAD_POOL_H_
#define NETLIB_NETLIB_THREAD_POOL_H_

#include <functional>
#include <vector>
#include <deque>

#include <netlib/non_copyable.h>
#include <netlib/mutex.h>
#include <netlib/condition.h>

namespace netlib
{

class ThreadPool: public NonCopyable
{
public:
	using Task = std::function<void()>;

	explicit ThreadPool(int thread_number, const Task &initial_task);
	~ThreadPool();

	// Must be called before Start().
	// The thread_number_ and initial_task shouldn't change once we create
	// the thread pool, so we don't expose set_*() for them. We can change the
	// task_queue_'s size() in the lifetime of thread pool.
	void set_max_queue_size(int max_queue_size)
	{
		max_queue_size_ = max_queue_size;
	}

	// Create thread_number_ threads and start all threads.
	void Start();
	// Run task() if thread_number_ is 0; otherwise add task into task queue.
	// Usually should be called immediately after Start() since Start() calls
	// RunInThread() and RunInThread() will block in GetAndRemoveTask()
	// if we don't RunOrAddTask().
	void RunOrAddTask(const Task &task);
	// TODO: C++11 `void RunOrAddTask(Task &&task);`
	// Stop all threads and call Join() for all threads.
	// After this function call, all threads can't run again.
	void Stop();

private:
	// The start function of thread. Start() -> RunInThread().
	void RunInThread();
	// Get task from task queue and remove the got task.
	// Called in RunInThread()'s while(running_) loop since we should assign
	// new task to thread after current task() return when the thread is running.
	Task GetAndRemoveTask();
	// Check whether task_queue_ is full, that is, task_queue_.size() >= max_queue_size_.
	// max_queue_size_ is set by user. Called in RunOrAddTask() when adding task.
	bool IsFull() const;

	const int thread_number_; // The number of threads in thread pool.
	std::vector<Thread*> thread_pool_; // Store thread_number_ threads' pointer.
	Task initial_task_; // The first task(i.e., function) that thread will finish.
	bool running_; // Indicate the status of all threads.
	mutable MutexLock mutex_; // Protect Condition and task queue.
	std::deque<Task> task_queue_;
	int max_queue_size_;
	// These two condition variables control the add/remove task into/from
	// task_queue. Used in three functions:
	// 1.	GetAndRemoveTask(): `not_empty_.Wait()` -> `not_full_.Notify()`
	// 2.	RunOrAddTask(): `not_full_.Wait()` -> `not_empty_.Notify()`
	// 3.	~Dtor() -> Stop(): `not_empty_.NotifyAll()`. Wakeup all threads that
	//		`not_empty_.Wait()` in GetAndRemoveTask() and stop them.
	Condition not_empty_; // Wait() when task_queue_ is empty.
	Condition not_full_; // Wait() when task_queue_ if full.
};

}

#endif // NETLIB_NETLIB_THREAD_POOL_H_
