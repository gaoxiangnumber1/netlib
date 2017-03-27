#include <stdio.h>
#include <memory>

using namespace std;

class Test
{
public:
	Test()
	{
		printf("Ctor\t");
	}
	~Test()
	{
		printf("Dtor\t");
	}
};

int main()
{
	printf("Stack Object without Dtor call.\n");
	{
		Test test;
	}
	printf("\nEnd\n");

	printf("Stack Object with Dtor call.\n");
	{
		Test test;
		test.~Test();
	}
	printf("\nEnd\n");

	printf("Heap Object without Dtor call.\n");
	{
		Test *test = new Test();
		test = test; // No warning, please.
	}
	printf("\nEnd\n");

	printf("Heap Object with Dtor call.\n");
	{
		Test *test = new Test();
		test->~Test();
	}
	printf("\nEnd\n");

	printf("Heap Object managed by smart_ptr, without Dtor() call.\n");
	{
		shared_ptr<Test> test(new Test());
	}
	printf("\nEnd\n");

	printf("Heap Object, managed by smart_ptr, with Dtor() call.\n");
	{
		shared_ptr<Test> test(new Test());
		(*test).~Test();
	}
	printf("\nEnd\n");
}
/*
Stack Object without Dtor call.
Ctor	Dtor
End
Stack Object with Dtor call.
Ctor	Dtor	Dtor
End
Heap Object without Dtor call.
Ctor
End
Heap Object with Dtor call.
Ctor	Dtor
End
Heap Object managed by smart_ptr, without Dtor() call.
Ctor	Dtor
End
Heap Object, managed by smart_ptr, with Dtor() call.
Ctor	Dtor	Dtor
End
*/
