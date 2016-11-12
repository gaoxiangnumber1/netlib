#include <netlib/thread_pool.h>

#include <netlib/thread.h>

using std::bind;
using std::vector;
using netlib::ThreadPool;

ThreadPool::ThreadPool(int thread_number, const Task &initial_task):
	thread_number_(thread_number),
	initial_task_(initial_task),
	running_(false),
	mutex_(),
	max_queue_size_(0),
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

// Create thread_number_ threads and start all threads.
void ThreadPool::Start()
{
	assert(thread_pool_.empty() == true);

	running_ = true;
	if(thread_number_ == 0 && initial_task_)
	{
		initial_task_();
	}
	thread_pool_.reserve(thread_number_);
	for(int index = 0; index < thread_number_; ++index)
	{
		// Create thread_number_ threads and start all threads.
		thread_pool_.push_back(new Thread(bind(&ThreadPool::RunInThread, this)));
		thread_pool_[index]->Start();
	}
}

// Run task directly when thread number is 0; otherwise add task into task queue
// (block when the task_queue is full: `not_full_.Wait()`) and wakeup threads
// by `not_empty_.Notify();` that wait on the not_empty_ Condition
// since the task queue must not be empty now.
void ThreadPool::RunOrAddTask(const Task &task)
{
	assert(running_ == true);
	if(thread_number_ == 0) // If thread_number_ = 0, run task() directly.
	{
		task();
	}
	else
	{
		MutexLockGuard lock(mutex_);
		while(IsFull() == true) // The task queue is full, can't add task.
		{
			not_full_.Wait(); // Wait until remove(GetTask()) task from task_queue_.
		}
		assert(IsFull() == false);
		// Add task into task queue and wakeup threads waiting on not_empty_.
		task_queue_.push_back(task);
		// TODO: Why notify() each time we add a task, rather than only notify()
		// when task_queue_.size() change from 0 to 1???
		not_empty_.Notify();
	}
}

// Stop all threads and call Join() for all threads.
void ThreadPool::Stop()
{
	{
		MutexLockGuard lock(mutex_);
		// Once set running_ to false, we can't set it to true. That is, all threads can't
		// run again.
		running_ = false;
		not_empty_.NotifyAll(); // TODO: what's use?
	}
	// Call Thread::Join() for every thread.
	for(vector<Thread*>::iterator it = thread_pool_.begin(); it != thread_pool_.end(); ++it)
	{
		(*it)->Join();
	}
}

// The start function of thread: first call initial_task if any; then indefinitely get
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
// `not_full_.Notify();` that wait on the not_full_ Condition
// since the queue must not be full now.
ThreadPool::Task ThreadPool::GetAndRemoveTask()
{
	MutexLockGuard lock(mutex_);
	// TODO: Study spurious wakeup! Always use a while-loop, due to spurious wakeup.
	while(task_queue_.empty() == true && running_ == true)
	{
		// Wait when the thread is in running state but has no task.
		not_empty_.Wait();
	}
	Task task;
	if(task_queue_.empty() == false && running_ == true)
	{
		task = task_queue_.front();
		task_queue_.pop_front();
		// Wakeup threads that wait on not_full_ condition since we have remove
		// one task and the task queue must not be empty.
		// TODO: Why Notify() each time we remove a task? rather than from
		// max_queue_size_ -> max_queue_size_ - 1?
		if(max_queue_size_ > 0)
		{
			not_full_.Notify();
		}
	}
	return task;
}

// Return true if the task queue if full(i.e., can't add more task).
bool ThreadPool::IsFull() const
{
	// Always called after `MutexLockGuard lock(mutex_);`
	mutex_.AssertLockedByThisThread();
	return (max_queue_size_ > 0) &&
	       (static_cast<int>(task_queue_.size()) >= max_queue_size_);
}
