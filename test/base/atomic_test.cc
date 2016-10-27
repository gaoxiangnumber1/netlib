#include <base/atomic.h>

#include <assert.h>

using netlib::AtomicInt32;
using netlib::AtomicInt64;

int main()
{
	{
		AtomicInt32 number1;
		assert(number1.Get() == 0);

		number1.Add(5);
		assert(number1.Get() == 5);

		number1.Set(10);
		assert(number1.Get() == 10);

		number1.Increment();
		assert(number1.Get() == 11);

		number1.Decrement();
		assert(number1.Get() == 10);

		assert(number1.GetAndAdd(10) == 10);
		assert(number1.Get() == 20);

		assert(number1.GetAndSet(30) == 20);
		assert(number1.Get() == 30);

		assert(number1.GetAndIncrement() == 30);
		assert(number1.Get() == 31);

		assert(number1.GetAndDecrement() == 31);
		assert(number1.Get() == 30);

		assert(number1.AddAndGet(-10) == 20);
		assert(number1.Get() == 20);

		assert(number1.IncrementAndGet() == 21);
		assert(number1.Get() == 21);

		assert(number1.DecrementAndGet() == 20);
		assert(number1.Get() == 20);
	}

	{
		AtomicInt64 number2;
		assert(number2.Get() == 0);

		number2.Add(5);
		assert(number2.Get() == 5);

		number2.Set(10);
		assert(number2.Get() == 10);

		number2.Increment();
		assert(number2.Get() == 11);

		number2.Decrement();
		assert(number2.Get() == 10);

		assert(number2.GetAndAdd(10) == 10);
		assert(number2.Get() == 20);

		assert(number2.GetAndSet(30) == 20);
		assert(number2.Get() == 30);

		assert(number2.GetAndIncrement() == 30);
		assert(number2.Get() == 31);

		assert(number2.GetAndDecrement() == 31);
		assert(number2.Get() == 30);

		assert(number2.AddAndGet(-10) == 20);
		assert(number2.Get() == 20);

		assert(number2.IncrementAndGet() == 21);
		assert(number2.Get() == 21);

		assert(number2.DecrementAndGet() == 20);
		assert(number2.Get() == 20);
	}

	return 0;
}

