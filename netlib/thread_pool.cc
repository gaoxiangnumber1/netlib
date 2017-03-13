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

// Create thread_number_ threads and start all threads.
void ThreadPool::Start()
{
	assert(running_ == false && static_cast<int>(thread_pool_.size()) == thread_number_);

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
// The start function of thread: first call initial_task_ if any; then indefinitely get
// task from task_queue and run until running_ is false.
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
// Get the first task from task queue(block when the task queue is empty:
// `not_empty_.Wait();`), then remove this task and wakeup threads by
// `not_full_.Notify();` that wait on the not_full_ Condition.
// NOTE: Not return `Task&` since we delete(pop_front()) the task from task queue.
ThreadPool::Task ThreadPool::GetAndRemoveTask()
{
	MutexLockGuard lock(mutex_);
	// TODO: Study spurious wakeup! Always use a while-loop, due to spurious wakeup.
	// NOTE: Must first check whether is in running state, otherwise the thread may
	// already Join()ed, so there is no sense to check task_queue_.
	while(running_ == true && task_queue_.empty() == true)
	{
		// Wait when the thread is in running state but has no task.
		not_empty_.Wait();
		// Be notified when:
		// 1.	New task is added by RunOrAddTask() into task_queue_, return valid task.
		// 2.	Call Stop() and running_ is false now, return invalid task(null).
	}
	Task task;
	if(running_ == true && task_queue_.empty() == false)
	{
		task = task_queue_.front();
		task_queue_.pop_front();
		// Wakeup thread that wait on not_full_ condition
		// each time we remove one task for efficiency.
		not_full_.Notify();
	}
	return task;
}

// Run task directly when thread number is 0; otherwise add task into task queue
// (block when the task_queue is full: `not_full_.Wait()`) and then wakeup threads
// by `not_empty_.Notify();`.
void ThreadPool::RunOrAddTask(const Task &task)
{
	assert(running_ == true); // NOTE: Use assertion for invariant.
	if(thread_number_ == 0) // If thread_number_ = 0, run task() directly.
	{
		task();
		return;
	}
	MutexLockGuard lock(mutex_);
	while(IsTaskQueueFull() == true) // The task queue is full, can't add task.
	{
		not_full_.Wait(); // Wait until remove task from task_queue_.
	}
	assert(IsTaskQueueFull() == false);
	// Add task into task queue and wakeup threads waiting on not_empty_.
	task_queue_.push_back(task);
	// Notify() each time we add a task, rather than only notify() when
	// task_queue_.size() change from 0 to 1 for efficiency.
	not_empty_.Notify();
}
// Return true if the task queue if full(i.e., can't add more task).
bool ThreadPool::IsTaskQueueFull() const
{
	// Always called after `MutexLockGuard lock(mutex_);`
	mutex_.AssertLockedByThisThread();
	return (max_queue_size_ > 0) &&
	       (static_cast<int>(task_queue_.size()) >= max_queue_size_);
}
