#ifndef NETLIB_SRC_BASE_ATOMIC_H_
#define NETLIB_SRC_BASE_ATOMIC_H_

#include <stdint.h> // int32_t, int64_t

#include <base/non_copyable.h> // netlib::NonCopyable class.

namespace netlib
{
namespace detail
{
template<typename T>
class AtomicInteger: public netlib::NonCopyable // TODO: What use of this class???
{
public:
	AtomicInteger(): value_(0) {}

	// 1. Perform no operation; Return current value.
	// Implement by: __atomic_load_n
	T Get()
	{
		// Implements an atomic load operation and returns the contents of *ptr.
		return __atomic_load_n(&value_, __ATOMIC_SEQ_CST);
		// When implementing patterns for these built-in functions, the memory order
		// parameter can be ignored as long as the pattern implements the most restrictive
		// __ATOMIC_SEQ_CST memory order. Any of the other memory orders execute
		// correctly with this memory order but they may not execute as efficiently as
		// they could with a more appropriate implementation of the relaxed requirements.
	}

	// 2. Perform the operation suggested by the name; Return no value.
	// Implement by: __atomic_fetch_add(discard return value!), __atomic_store_n(set)
	void Add(T value)
	{
		// There is no atomic built-in gcc function that perform addition
		// without returning value. So we just discard the return value.
		__atomic_fetch_add(&value_, value, __ATOMIC_SEQ_CST);
	}
	void Set(T new_value)
	{
		__atomic_store_n(&value_, new_value, __ATOMIC_SEQ_CST);
	}
	void Increment() // Delegate work to Add(1)
	{
		Add(1);
	}
	void Decrement() // Delegate work to Add(-1)
	{
		Add(-1);
	}

	// 3.1 Perform the operation suggested by the name; Return the old value.
	// Implement by: __atomic_fetch_add(addition), __atomic_exchange_n(set)
	T GetAndAdd(T value)
	{
		// Perform the add operation and return the value that had previously been in *ptr.
		return __atomic_fetch_add(&value_, value, __ATOMIC_SEQ_CST);
	}
	T GetAndSet(T new_value)
	{
		// Implement an atomic exchange operation: write val into *ptr,
		// and returns the previous contents of *ptr.
		return __atomic_exchange_n(&value_, new_value, __ATOMIC_SEQ_CST);
	}
	T GetAndIncrement() // Delegate work to GetAndAdd(1).
	{
		return GetAndAdd(1);
	}
	T GetAndDecrement() // Delegate work to GetAndAdd(-1).
	{
		return GetAndAdd(-1);
	}

	// 3.2 Perform the operation suggested by the name; Return the new value.
	// Implement by: __atomic_add_fetch(addition), no_gcc_built_in_function(set)
	T AddAndGet(T value)
	{
		// TODO: Why cs `return GetAndAdd(value) + value;`
		return __atomic_add_fetch(&value_, value, __ATOMIC_SEQ_CST);
	}
	// T SetAndGet(T new_value);
	// GCC offer no corresponding built-in atomic function. This is reasonable
	// since we pass the new_value and we already know the Get() result after Set(),
	// that is the same as new_value. So, there is no need to offer SetAndGet().
	T IncrementAndGet() // Delegate work to AddAndGet(1)
	{
		return AddAndGet(1);
	}
	T DecrementAndGet() // Delegate work to AddAndGet(-1)
	{
		return AddAndGet(-1);
	}

private:
	volatile T value_; // TODO: when to use `volatile`
};
}

using AtomicInt32 = detail::AtomicInteger<int32_t>;
using AtomicInt64 = detail::AtomicInteger<int64_t>;
}
#endif // NETLIB_SRC_BASE_ATOMIC_H_
