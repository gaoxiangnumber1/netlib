#include <netlib/thread.h>
#include <netlib/thread_pool.h>
#include <netlib/count_down_latch.h>

#include <unistd.h> // sleep()

using std::bind;
using netlib::Thread;
using netlib::ThreadPool;
using netlib::CountDownLatch;

void InitialTask()
{
	printf("%d: 0.\n", Thread::ThreadId());
	sleep(1);
}

void Print(int number)
{
	printf("%d: %d\n", Thread::ThreadId(), number);
}

void Test(int max_queue_size, int thread_number = 5)
{
	printf("ThreadPool: thread_number = %d, max_queue_size = %d\n",
	       thread_number, max_queue_size);
	ThreadPool pool(thread_number, InitialTask, max_queue_size);
	pool.Start();

	printf("Adding\n");
	for(int index = 1; index <= 100; ++index)
	{
		pool.RunOrAddTask(bind(Print, index));
	}
	printf("Done\n");

	CountDownLatch latch(1);
	pool.RunOrAddTask(bind(&CountDownLatch::CountDown, &latch));
	latch.Wait();
	pool.Stop();
}

int main()
{
	Test(0);
	//Test(10);
	//Test(20);
	//Test(40);
	//Test(100);
}
