#include <netlib/thread.h>

#include <assert.h> // assert()
#include <sys/syscall.h> // syscall()
#include <unistd.h> // syscall()

#include <netlib/logging.h>

using netlib::Thread;

int Thread::created_number_(0);
Thread::Thread(const ThreadFunction &function)
	: started_(false),
	  joined_(false),
	  pthread_id_(0),
	  thread_id_(0), // Set after pthread_create().
	  function_(function)
{
	++created_number_;
}

Thread::~Thread()
{
	if(started_ == true && joined_ == false)
	{
		assert(pthread_detach(pthread_id_) == 0);
	}
}

namespace netlib
{
struct ThreadData // Store the arguments that need passed to pthread_create.
{
	using ThreadFunction = Thread::ThreadFunction;
	// All data members are references: bind to the data member in Thread object.
	const ThreadFunction &function_;
	int &thread_id_;

	ThreadData(const ThreadFunction &function, int &thread_id)
		: function_(function),
		  thread_id_(thread_id)
	{}

	void RunInThread()
	{
		// 1. Set thread id.
		thread_id_ = Thread::ThreadId();
		// 2. Call thread work function.
		function_();
	}
};
void *StartThread(void *object) // Thread start function passed to pthread_create.
{
	ThreadData *data = static_cast<ThreadData*>(object);
	data->RunInThread();
	delete data; // NOTE: delete data!
	return nullptr;
}
}
using netlib::ThreadData;
void Thread::Start()
{
	assert(started_ == false);
	started_ = true;
	// TODO: C++11 move(function_)
	ThreadData *data = new ThreadData(function_, thread_id_);
	if(pthread_create(&pthread_id_, NULL, &netlib::StartThread, data) != 0)
	{
		started_ = false;
		delete data; // NOTE: delete data!
		LOG_FATAL("pthread_create: FATAL");
	}
}

// Every thread has its own instance of __thread variable.
__thread int t_cached_thread_id = 0; // The thread id in the kernel, not the pthread_t.
int Thread::ThreadId() // Return the cached thread-id.
{
	if(t_cached_thread_id == 0) // If not cached yet.
	{
		// The return type of syscall() is long int.
		t_cached_thread_id = static_cast<int>(::syscall(SYS_gettid));
	}
	return t_cached_thread_id;
}

// Since we cache the thread id in t_cached_thread_id, after fork(2), the child
// process can see the cached thread id, but it is not its own thread id. Thus, we
// empty the cached thread id for child process and re-cache the right thread id
// in the child fork handler before returning from fork(2),
void ChildForkHandler()
{
	t_cached_thread_id = 0;
	Thread::ThreadId();
}
struct ForkHandler
{
	ForkHandler()
	{
		assert(pthread_atfork(NULL, NULL, &ChildForkHandler) == 0);
	}
};
// C++ guarantee that the global object's constructs is finished before enter main().
ForkHandler fork_handler; // Global object.

void Thread::Join()
{
	assert(started_ == true && joined_ == false);
	joined_ = true;
	assert(pthread_join(pthread_id_, NULL) == 0);
}
