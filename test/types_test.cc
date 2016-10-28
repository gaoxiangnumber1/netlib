#define NDEBUG

#include <base/types.h>

using netlib::string;
using netlib::up_cast;
using netlib::down_cast;

class TestParent
{
public:
	virtual ~TestParent() {}
	int value_parent_ = 0;
};

class Test: public TestParent
{
public:
	virtual ~Test() {}
	int value_ = 0;
};

class TestChild: public Test
{
public:
	~TestChild() {}
	int value_child_ = 0;
};

int main()
{
	// Test netlib::string.
	string string1 = "gao", string2 = "xiang";
	string string3 = string1 + string2;

	// Test up_cast
	Test test, *test_ptr = &test;
	up_cast<TestParent*>(test_ptr);
	up_cast<const Test*>(test_ptr);

	// Test down_cast
	down_cast<TestChild*>(test_ptr);

	return 0;
}
