#ifndef NETLIB_SRC_EVENTLOOP_H_
#define NETLIB_SRC_EVENTLOOP_H_

#include <sys/types.h> // pid_t

#include <non_copyable.h>

namespace netlib
{

class Channel;
class Poller;

class EventLoop: public netlib::NonCopyable
{
public:
	EventLoop(); // Check whether satisfy `one loop per thread`.
	~EventLoop();

	void Loop();
	void AssertInLoopThread();
	void Quit();

	void UpdateChannel(Channel *channel);

	// Return the object that hold by current thread.
	static EventLoop *GetEventLoopOfCurrentThread();

private:
	using ChannelVector = std::vector<Channel*>;

	bool looping_; // Atomic
	bool quit_; // Atomic
	const pid_t thread_id_;
	std::unique_ptr<Poller> poller_;
	ChannelVector active_channel_;
};

}
#endif // NETLIB_SRC_EVENTLOOP_H_
