#ifndef NETLIB_SRC_CURRENT_THREAD_H_
#define NETLIB_SRC_CURRENT_THREAD_H_

#include <stdint.h> // Header for `int64_t`

namespace netlib
{
namespace current_thread
{
// Used with `-fprofile-arcs` to collect branch information
// Expect condition to happen.
#define LIKELY(condition) __builtin_expect(!!(condition), 1)
// Expect condition not happen.
#define UNLIKELY(condition) __builtin_expect(!!(condition), 0)

extern __thread int t_cached_thread_id;
extern __thread char t_thread_id_string[32];
extern __thread int t_thread_id_string_length;
extern __thread const char *t_thread_name;

void CacheThreadId(); // TODO: How to implement?

inline int ThreadId() // Return the cached thread-id.
{
	if(UNLIKELY(t_cached_thread_id == 0)) // Expect t_cached_thread_id != 0
	{
		CacheThreadId(); // If not cached yet.
	}
	return t_cached_thread_id;
}

// For logging: return the string corresponding to current thread's id.
inline const char* ThreadIdString()
{
	return t_thread_id_string;
}

// For logging: return the string's length corresponding to current thread's id.
inline int ThreadIdStringLength()
{
	return t_thread_id_string_length;
}

inline const char* Name() // Return thread's name
{
	return t_thread_name;
}

bool IsMainThread(); // TODO: How to implement?

void SleepUsec(int64_t usec); // TODO: How to implement?
}
}

#endif // NETLIB_SRC_CURRENT_THREAD_H_
