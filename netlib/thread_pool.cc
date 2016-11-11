#include <netlib/thread_pool.h>

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

ThreadPool::~ThreadPool()
{
	if(running_ == true)
	{
		Stop();
	}
}

void ThreadPool::Start(int thread_number)
{
	assert(thread_pool_.empty() == true);

	running_ = true;
	thread_pool_.reserve(thread_number);
	for(int index = 0; index < thread_number; ++index)
	{
		// Create thread_number threads and let all threads start.
		thread_pool_.push_back(new Thread(&ThreadPool::RunInThread, this));
		thread_pool_[index].Start();
	}
	if(thread_number == 0 && initial_task_)
	{
		initial_task_();
	}
}

void ThreadPool::Stop()
{
	{
		MutexLockGuard lock(mutex_);
		running_ = false;
		not_empty_.NotifyAll();
	}
	for(vector<Thread*>::iterator it = thread_pool_.begin(); it != thread_pool_.end(); ++it)
	{
		(*it)->Join();
	}
}

void ThreadPool::IsFull() const
{
	mutex_.AssertLockedByThisThread();
	return (max_queue_size_ > 0) && (task_queue_.size() >= max_queue_size_);
}

void ThreadPool::Run(const Task &task)
{
	if(thread_pool_.empty())
	{
		task();
	}
	else
	{
		MutexLockGuard lock(mutex_);
		while(IsFull() == true)
		{
			not_full_.Wait();
		}
		assert(IsFull() == false);
		task_queue_.push_back(task);
		not_empty_.Notify();
	}
}

Task ThreadPool::GetTask()
{
	MutexLockGuard lock(mutex_);
	// Always use a while-loop, due to spurious wakeup.
	while(task_queue_.empty() == true && running_ == true)
	{
		not_empty_.Wait();
	}
	Task task;
	if(task_queue_.empty() == false)
	{
		task = task_queue_.front();
		task_queue_.pop_front();
		if(max_queue_size_ > 0)
		{
			not_full_.Notify();
		}
	}
	return task;
}

void ThreadPool::RunInThread()
{
	if(initial_task_)
	{
		initial_task_();
	}
	while(running_ == true)
	{
		Task task(GetTask());
		if(task)
		{
			task();
		}
	}
}
