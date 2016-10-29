#ifndef NETLIB_SRC_THREAD_H_
#define NETLIB_SRC_THREAD_H_

namespace netlib
{
class Thread: public NonCopyable
{
public:
	using ThreadFunction = std::function<void()>;

	explicit Thread(const ThreadFunction&, const string &name = string());
	explicit Thread(ThreadFunction&&, const string &name = string());
	~Thread();

	void Start();
	int Join(); // return pthread_join()

	bool started() const
	{
		return started_;
	}
	pthread_t thread_id() const
	{
		return thread_id_;
	}
	pid_t pid() const
	{
		return *pid_;
	}
	const string &name() const
	{
		return name_;
	}

	static int created_number()
	{
		return created_number_.load();
	}

private:
	void SetDefaultName();
	bool started_;
	bool joined_;
	pthread_t thread_id_;
	std::shared_ptr<pid_t> pid_;
	ThreadFunction function_;
	string name_;
	static atomic<int32_t> created_number_;
};
}

#endif // NETLIB_SRC_THREAD_H_
