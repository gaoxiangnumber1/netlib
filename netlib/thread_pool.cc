#include <netlib/thread_pool.h>
#include <netlib/thread.h>

using std::bind;
using netlib::ThreadPool;

ThreadPool::ThreadPool(const int thread_number,
                       const Task &initial_task,
                       const int max_queue_size):
	thread_number_(thread_number),
	thread_pool_(thread_number_),
	initial_task_(initial_task),
	running_(false),
	mutex_(),
	max_queue_size_(max_queue_size),
	not_empty_(mutex_),
	not_full_(mutex_)
{}

// Stop all threads if is running.
ThreadPool::~ThreadPool()
{
	if(running_ == true)
	{
		Stop();
	}
	// All data members(including `MutexLock mutex_`) are about to Destruct!
}
// Stop all threads and call Join() for all threads.
void ThreadPool::Stop()
{
	// Once set to false, we can't set it to true, i.e., all threads can't run again.
	running_ = false;

	// Use as short critical section as possible. {critical section;} non-critical;
	{
		MutexLockGuard lock(mutex_);
		// Reason for only wakeup not_empty_:
		// 1.	not_empty_.Wait() <- GetAndRemoveTask() <- RunInThread() <- Child thread
		//		start function. ONLY child thread can Wait() on not_empty_.
		// 2.	not_full_.Wait() <- RunOrAddTask() <- Main thread called. ONLY main thread
		//		can Wait() on not_full_.
		// 3.	Stop() can be called by Main thread ONLY.
		// 4.	One thread can execute one function at a time: when in Stop(), main thread
		//		can't Wait() on not_full_, thus no thread Wait() on not_full_ now.
		// Summary: Main thread wakeup child thread to exit.
		not_empty_.NotifyAll();
	}
	for(int index = 0; index < thread_number_; ++index)
	{
		thread_pool_[index]->Join();
	}
}

void ThreadPool::Start()
{
	assert(running_ == false);

	running_ = true;
	if(thread_number_ == 0 && initial_task_)
	{
		initial_task_();
	}
	for(int index = 0; index < thread_number_; ++index)
	{
		thread_pool_[index] = new Thread(bind(&ThreadPool::RunInThread, this));
		thread_pool_[index]->Start();
	}
}
// The start function of thread.
void ThreadPool::RunInThread()
{
	// First run the initial_task_ if any.
	if(initial_task_)
	{
		initial_task_();
	}
	// Then indefinitely get(and remove) task from task queue and run it
	// until running_ flag is set false by Stop().
	while(running_ == true)
	{
		Task task(GetAndRemoveTask());
		if(task)
		{
			task();
		}
	}
}
// NOTE: Not return `Task&` since we delete(pop_front()) the task from task queue.
ThreadPool::Task ThreadPool::GetAndRemoveTask()
{
	MutexLockGuard lock(mutex_);
	// Always use a while-loop, due to spurious wakeup.
	while(running_ == true && task_queue_.empty() == true)
	{
		not_empty_.Wait();
		// Wait() return when:
		// 1.	New task is added by RunOrAddTask() into task_queue_, return valid task.
		// 2.	Call Stop() and running_ is false now, return invalid task(null).
	}
	Task task;
	if(running_ == true)
	{
		task = task_queue_.front();
		task_queue_.pop_front();
		// Wakeup main thread each time we remove one task for efficiency.
		not_full_.Notify();
	}
	return task;
}

void ThreadPool::RunOrAddTask(const Task &task)
{
	assert(running_ == true); // NOTE: Use assertion for invariant.
	if(thread_number_ == 0)
	{
		task();
		return;
	}
	MutexLockGuard lock(mutex_);
	while(IsTaskQueueFull() == true) // The task queue is full, can't add task.
	{
		not_full_.Wait();
	}
	assert(IsTaskQueueFull() == false);
	// Add task into task queue and wakeup child thread.
	task_queue_.push_back(task);
	not_empty_.Notify();
}
// Return true if the task queue if full(i.e., can't add more task).
bool ThreadPool::IsTaskQueueFull() const
{
	mutex_.AssertLockedByThisThread();
	return (max_queue_size_ > 0) &&
	       (static_cast<int>(task_queue_.size()) >= max_queue_size_);
}
