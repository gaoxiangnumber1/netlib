#ifndef NETLIB_NETLIB_TIMER_QUEUE_H_
#define NETLIB_NETLIB_TIMER_QUEUE_H_

#include <set>
#include <vector>

#include <netlib/callback.h>
#include <netlib/channel.h>
#include <netlib/non_copyable.h>
#include <netlib/time_stamp.h>

namespace netlib
{

class EventLoop;
class Timer;
class TimerId;

// Interface:
// Ctor -> -CreateTimerFd -> -HandleRead.
//			-HandleRead -> -ReadTimerFd -> -GetAndRemoveExpiredTimer -> -Refresh.
//						-Refresh -> -InsertIntoActiveTimerSet -> -SetExpiredTime.
// Dtor.
// AddTimer -> -AddTimerInLoop -> -InsertIntoActiveTimerSet -> SetExpiredTime.
// CancelTimer -> -CancelTimerInLoop.

class TimerQueue: public NonCopyable
{
public:
	TimerQueue(EventLoop *owner_loop);
	~TimerQueue();

	TimerId AddTimer(const TimerCallback &callback,
	                 const TimeStamp &expired_time,
	                 double interval);
	void CancelTimer(const TimerId &timer_id);

private:
	using TimerVector = std::vector<Timer*>;
	using ExpirationTimerPair = std::pair<TimeStamp, Timer*>;
	using ExpirationTimerPairSet = std::set<ExpirationTimerPair>;

	int CreateTimerFd();
	void HandleRead(); // Timer expires -> timer_fd_ is readable
	void ReadTimerFd(const TimeStamp &time_stamp);
	void GetAndRemoveExpiredTimer(const TimeStamp &now);
	void Refresh(const TimeStamp &now); // Restart or delete expired timer
	bool InsertIntoActiveTimerSet(Timer *timer); // Return true if `timer` will expire first.
	void SetExpiredTime(const TimeStamp &expiration);
	void AddTimerInLoop(Timer *timer);
	void CancelTimerInLoop(TimerId timer_id);

	EventLoop *owner_loop_;
	const int timer_fd_;
	Channel timer_fd_channel_;
	ExpirationTimerPairSet active_timer_set_;
	TimerVector expired_timer_vector_;
	std::set<int64_t> canceling_timer_sequence_set_;
};

}

#endif // NETLIB_NETLIB_TIMER_QUEUE_H_
