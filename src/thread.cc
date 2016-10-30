#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // _GNU_SOURCE

#include <thread.h>

#include <assert.h> // assert()
#include <unistd.h>
#include <sys/syscall.h> // syscall()
#include <time.h> // nanosleep()

#include <logging.h> // LOG_FATAL()

using std::shared_ptr;
using std::weak_ptr;
using std::move;
using std::atomic;

namespace netlib
{

const int kMicroSecondsPerSecond = 1000 * 1000;

namespace detail
{
struct ThreadData
{
	using ThreadFunction = netlib::Thread::ThreadFunction;

	ThreadFunction function_;
	weak_ptr<pid_t> weak_thread_id_;

	ThreadData(const ThreadFunction &function,
	           const shared_ptr<pid_t> &shared_thread_id)
		: function_(function),
		  weak_thread_id_(shared_thread_id)
	{}

	void RunInThread()
	{
		// 1. Set thread id.
		pid_t thread_id = netlib::ThreadId();
		// If the number of shared_ptrs that share ownership with weak_ptr is 0,
		// return a null shared_ptr; otherwise return a shared_ptr to the object
		// to which weak_ptr points.
		shared_ptr<pid_t> shared_thread_id = weak_thread_id_.lock();
		if(shared_thread_id) // If the object still exist, now use_count() >= 2.
		{
			*shared_thread_id = thread_id; // Set new value.
			// p.reset():
			// 1. Make p stop sharing current object(i.e., --use_count()) and set p to null.
			// 2. Then if use_count() == 0, frees p's existing object.
			shared_thread_id.reset();
		}
		// 2. Call thread work function.
		function_();
	}
};

void *StartThread(void *object)
{
	ThreadData *data = static_cast<ThreadData*>(object);
	data->RunInThread();
	delete data;
	return NULL;
}

}

// Every thread has it own instance of __thread variable.
__thread pid_t t_cached_thread_id = 0; // The thread id in the kernel, not the pthread_t.

pid_t ThreadId() // Return the cached thread-id.
{
	if(t_cached_thread_id == 0) // If not cached yet.
	{
		t_cached_thread_id = static_cast<pid_t>(::syscall(SYS_gettid));
	}
	return t_cached_thread_id;
}

void SleepUsec(int64_t usec)
{
	struct timespec ts = {0, 0};
	ts.tv_sec = static_cast<time_t>(usec / kMicroSecondsPerSecond); // seconds
	ts.tv_nsec = static_cast<long>(usec % kMicroSecondsPerSecond  *1000); // nanoseconds
	::nanosleep(&ts, NULL);
}

atomic<int32_t> Thread::created_number_(0);

Thread::Thread(const ThreadFunction &function)
	: started_(false),
	  joined_(false),
	  pthread_id_(0),
	  shared_thread_id_(new pid_t(0)),
	  function_(function)
{
	++created_number_;
}

Thread::Thread(ThreadFunction &&function)
	: started_(false),
	  joined_(false),
	  pthread_id_(0),
	  shared_thread_id_(new pid_t(0)),
	  function_(move(function))
{
	++created_number_;
}

Thread::~Thread()
{
	if(started_ == true && joined_ == false)
	{
		// By default, a thread's termination status is retained until we call pthread_join
		// for that thread. A thread's underlying storage can be reclaimed immediately
		// on termination if the thread has been detached by calling pthread_detach.
		// int pthread_detach(pthread_t tid);
		pthread_detach(pthread_id_);
	}
}

void Thread::Start()
{
	assert(started_ == false);
	started_ = true;
	// FIXME: move(function_)
	detail::ThreadData *data = new detail::ThreadData(function_, shared_thread_id_);

	// int pthread_create(pthread_t *ptid, const pthread_attr_t *attr,
	//                    void* (*fun) (void*), void *arg);
	// Return: 0 if OK, error number on failure
	// *ptid is set to the thread ID of the newly created thread.
	// attr = NULL: create a thread with the default attributes.
	// The newly created thread starts running at the address of the fun function.
	// arg is passed to fun.
	if(pthread_create(&pthread_id_, NULL, &detail::StartThread, data))
	{
		started_ = false;
		delete data;
		LOG_FATAL("Failed in pthread_create");
	}
}

int Thread::Join()
{
	assert(started_);
	assert(joined_ == false);
	joined_ = true;
	// pthread_join(ptid, NULL): wait for the specified thread without
	// retrieve the thread’s termination status.
	return pthread_join(pthread_id_, NULL);
}

}
