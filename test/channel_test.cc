#include <unistd.h> // getpid()

#include <map>

#include <netlib/logging.h>
#include <netlib/event_loop.h>

using std::bind;
using std::map;
using netlib::EventLoop;
using netlib::TimeStamp;

map<const char*, TimeStamp> message_time_map;
int g_count = 0;
EventLoop *g_loop;

void Print(const char* message)
{
	TimeStamp &last = message_time_map[message];
	TimeStamp now = TimeStamp::Now();
	printf("%s %s delay %f\n",
	       now.ToFormattedTimeString().c_str(),
	       message,
	       TimeDifferenceInSecond(now, last));
	last = now;
	if(++g_count == 30)
	{
		g_loop->Quit();
	}
}

int main()
{
	LOG_INFO("pid = %d", getpid());
	EventLoop loop;
	g_loop = &loop;
	message_time_map["RunEvery"] = TimeStamp::Now();
	loop.RunEvery(bind(Print, "RunEvery"), 1.0);
	loop.Loop();
}
