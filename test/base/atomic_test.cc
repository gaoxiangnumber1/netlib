#include <base/atomic.h>

#include <assert.h>

int main()
{
	{
		netlib::AtomicInt64 number1;
		assert(number1.Get() == 0);
		assert(number1.GetAndAdd(1) == 0);
		assert(number1.Get() == 1);
		assert(number1.AddAndGet(2) == 3);
		assert(number1.Get() == 3);
		assert(number1.IncrementAndGet() == 4);
		assert(number1.Get() == 4);
		number1.Increment();
		assert(number1.Get() == 5);
		assert(number1.AddAndGet(-3) == 2);
		assert(number1.GetAndSet(100) == 2);
		assert(number1.Get() == 100);
	}

	{
		netlib::AtomicInt32 number2;
		assert(number2.Get() == 0);
		assert(number2.GetAndAdd(1) == 0);
		assert(number2.Get() == 1);
		assert(number2.AddAndGet(2) == 3);
		assert(number2.Get() == 3);
		assert(number2.IncrementAndGet() == 4);
		assert(number2.Get() == 4);
		number2.Increment();
		assert(number2.Get() == 5);
		assert(number2.AddAndGet(-3) == 2);
		assert(number2.GetAndSet(100) == 2);
		assert(number2.Get() == 100);
	}

	return 0;
}

