#include <netlib/thread.h>
#include <netlib/thread_pool.h>
#include <netlib/count_down_latch.h>

using std::bind;
using netlib::Thread;
using netlib::ThreadPool;
using netlib::CountDownLatch;

void InitialTask()
{
	printf("%d: initial task\n", Thread::ThreadId());
}

void Print()
{
	printf("tid=%d task\n", Thread::ThreadId());
}

void PrintString(const char *str)
{
	printf("%s\n", str);
}

void Test(int max_queue_size)
{
	printf("Test ThreadPool with max queue size = %d\n", max_queue_size);
	ThreadPool pool(5, InitialTask);
	pool.set_max_queue_size(max_queue_size);
	pool.Start();

	printf("Adding\n");
	pool.RunOrAddTask(Print);
	pool.RunOrAddTask(Print);
	for(int index = 0; index < 100; ++index)
	{
		char buf[32];
		snprintf(buf, sizeof buf, "task %d", index);
		pool.RunOrAddTask(bind(PrintString, buf));
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
	Test(5);
	Test(10);
	Test(30);
	Test(100);
}
