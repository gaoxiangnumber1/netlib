#ifndef NETLIB_NETLIB_SINGLETON_H_
#define NETLIB_NETLIB_SINGLETON_H_

#include <assert.h> //  assert()
#include <pthread.h> // pthread_*

#include <netlib/non_copyable.h> // NonCopyable class.

namespace netlib
{

// Interface:
// Instance -> -Initialize

// Usage: `Test &test = Singleton<Test>::Instance();`

template<typename T>
class Singleton: public NonCopyable
{
public:
	static T &Instance()
	{
		// #include <pthread.h>
		// pthread_once_t initflag = PTHREAD_ONCE_INIT;
		// int pthread_once(pthread_once_t *initflag, void(*initfn)());
		// Return 0 if OK, error number on failure.
		// initflag must be a nonlocal variable(i.e., global or static) and initialized to
		// PTHREAD_ONCE_INIT.
		// If each thread calls pthread_once, the system guarantees that the initialization
		// routine, initfn, will be called only once, on the first call to pthread_once.
		pthread_once(&once_, &Singleton::Initialize);
		assert(value_ != nullptr);
		return *value_;
	}

private:
	Singleton();
	~Singleton();

	static void Initialize()
	{
		value_ = new T();
	}

	static pthread_once_t once_;
	static T *value_;
};

template<typename T>
pthread_once_t Singleton<T>::once_ = PTHREAD_ONCE_INIT;

template<typename T>
T *Singleton<T>::value_ = nullptr;

}

#endif // NETLIB_NETLIB_SINGLETON_H_
