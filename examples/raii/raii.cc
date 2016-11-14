#include <iostream>
#include <memory>

using namespace std;

class Test
{
public:
	Test()
	{
		cout << "Ctor()\n";
	}
	~Test()
	{
		cout << "Dtor()\n";
	}
};

int main()
{
	cout << "stack object, without dtor call.\n";
	{
		Test test;
	}
	cout << "End\n";

	cout << "stack object, with dtor call.\n";
	{
		Test test;
		test.~Test();
	}
	cout << "End\n";

	cout << "heap object, without dtor call.\n";
	{
		Test *test = new Test();
	}
	cout << "End\n";

	cout << "heap object, with dtor call.\n";
	{
		Test *test = new Test();
		test->~Test();
	}
	cout << "End\n";

	cout << "heap object, managed by smart_ptr, without dtor() call.\n";
	{
		shared_ptr<Test> test(new Test());
	}
	cout << "End\n";

	cout << "heap object, managed by smart_ptr, with dtor() call.\n";
	{
		shared_ptr<Test> test(new Test());
		(*test).~Test();
	}
	cout << "End\n";
}
/*
stack object, without dtor call.
Ctor()
Dtor()
End
stack object, with dtor call.
Ctor()
Dtor()
Dtor()
End
heap object, without dtor call.
Ctor()
End
heap object, with dtor call.
Ctor()
Dtor()
End
heap object, managed by smart_ptr, without dtor() call.
Ctor()
Dtor()
End
heap object, managed by smart_ptr, with dtor() call.
Ctor()
Dtor()
Dtor()
End
*/
