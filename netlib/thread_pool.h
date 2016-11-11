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
	void set_max_queue_size(int max_queue_size)
	{
		max_queue_size_ = max_queue_size;
	}

	void Start(int thread_number);
	void Stop();
	// Could block if max_queue_size_ > 0.
	void Run(const Task &task);
	// TODO: C++11 `void Run(Task &&task);`

private:
	bool IsFull() const;
	void RunInThread();
	Task GetTask();

	const int thread_number_; // The number of threads in thread pool.
	std::vector<Thread*> thread_pool_; // Store thread_number_ threads' pointer.
	Task initial_task_; // The first task(i.e., function) that thread will finish.
	bool running_; // Indicate the status of all threads.
	mutable MutexLock mutex_; // Protect Condition and task queue.
	std::deque<Task> task_queue_;
	int max_queue_size_;
	Condition not_empty_; // Wait() when task_queue_ is empty.
	Condition not_full_; // Wait() when task_queue_ if full.
};

}

#endif // NETLIB_NETLIB_THREAD_POOL_H_
